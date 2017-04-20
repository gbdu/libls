#include <stdio.h>
#include <limits.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include "libls.h"
#include "ls_store.h"

extern void (*ls_error)(char*)  ; 

index_item * lss_load_index(const char * list_wd, unsigned int * items);

#include "ls_deb.h"

void ls_debug_index(index_item i){
	printf("\n ------ ls_debug_index -----\n"
	       "id: %u\n"
	       "size_32: %u\n"
	       "flag_mask: %x\n",
	       i.id, i.size_32, i.flag_mask);
	printf("-------- end of ls_debug_index ------\n");
}
bool lss_mk_wd(const char * working_directory){
	mode_t directory_mode = S_IRWXU | S_IRWXG ; 
	if(mkdir(working_directory, directory_mode)){
		char err[PATH_MAX * 3] ;
		perror("File I/O error");
		sprintf(err, "Could not create list directory (%s) ",
				working_directory);
		ls_error(err);
		return 0;
	}
	else{
		return 1;
	}
}

int lss_store_index(const char * list_wd, index_item * index, size_t items){
	char index_path[PATH_MAX * 2] ;
	sprintf(index_path, "%s/index.lsx", list_wd);
	FILE * index_d = fopen(index_path, "wb") ;
	
	if(index_d == NULL ){
		char err[PATH_MAX * 3] ;
		sprintf(err, "Can't write index file(%s)", index_path);
		perror("File I/O error");
		ls_error(err);
		return -2 ;
	}
	
	if(SIZE_MAX < items){
		ls_error("Can't store this many items");
		return -1 ;
	}

	uint32_t items_32 = (size_t) items ;

	if(fwrite(&items_32, 4, 1, index_d) != 1){
		perror("I/O error");
		ls_error("Can't write index size");
		return -2;
	}
	
	if(items && fwrite(index, sizeof(index_item), items, index_d) != items){
		perror("Can't write index");
		ls_error("Can't write index");
		return -3;
	}

	fclose(index_d);
	return 0 ;
}

typedef struct lss_index_callback_data {
	index_item * index ;
	size_t item ; 
	uint32_t (*size_func)(el*, void**);
	void ** size_func_params ; 
} lss_index_callback_data ;

bool lss_add_el_to_index(el * e, void ** vargs){
	lss_index_callback_data * p = (lss_index_callback_data*) vargs;
		
	(p->index)[p->item].id = (uint32_t) e->id ;
	(p->index)[p->item].size_32 = p->size_func(e, p->size_func_params) ;
	if(!(p->index)[p->item].size_32){
		ls_error("Can't add empty element to index.");
	}
	(p->index)[p->item].flag_mask = e->flag_mask ;

	p->item ++ ;
	return 1;
}

index_item * 
lss_build_index(list * ls,
	uint32_t(*user_size_func)(el*, void**),
	void ** size_func_params){

	index_item * index = (index_item*)
		malloc(sizeof(index_item) * ls->size);

	// TODO: Why does valgrind cry when I remove this?
	memset(index, 0, sizeof(index_item) * ls->size);

	if(index == NULL){
		ls_error("Out of memory");
		return NULL ;
	}

	lss_index_callback_data p = {
		index,
		0,
		user_size_func,
		size_func_params
	};

	ls_foreach(ls, lss_add_el_to_index, (void**) &p);

	return index ;
}

typedef struct lss_store_el_callback_data {
	list * ls ;
	const char * list_wd ; 
	uint32_t (*size_func)(el *, void**) ;
	void ** size_func_params;
	bool success ; 
} lss_store_el_callback_data;

bool lss_store_el(el * e, void ** vargs){
	lss_store_el_callback_data * p = (lss_store_el_callback_data*) vargs ;

	char path[PATH_MAX+1] ;
	FILE * fp = NULL ;

	if(ls_is_element_sub(e)){ // This is a sub-list
		sprintf(path, "%s/%u", p->list_wd, e->id);
		list * sub_list = ls_get_sublist(p->ls, e->id);

		if(sub_list == NULL){
			char err[35] ; 
			sprintf(err, "Element (%d) is not a sub-list or "
					"doesn't exist", e->id);
			ls_error(err);
			p->success = 0;
			return 0 ;
		}
		lss_store_list(sub_list, path, p->size_func,
				p->size_func_params);
		return 1;
	} 
	else { // Else, this is a regular element 
		sprintf(path, "%s/%u.ls", p->list_wd, e->id);
		fp = fopen(path, "wb");
		
		if(fp == NULL){
			char err[PATH_MAX*2];
			perror("File I/O error");
			sprintf(err, "Could not open file (%s) for writing", path);
			p->success = 0;
			return 0;
		}

		size_t el_size = p->size_func(e, (void**) p->size_func_params) ;
		
		if(fwrite(e->data, el_size, 1, fp) != 1){
			char err[PATH_MAX * 3 ];
			perror("File I/O error");
			sprintf(err, "Could not write (%d) bytes to (%s)",
					el_size, path);
			p->success = 0;
			return 0 ; 
		}
		p->success = 1;
		fclose(fp);
		return p->success ? 0 : -5 ;
	}
}

int 
lss_build_save_index (list * ls,
		const char * list_wd,
		uint32_t (*size_func)(el*, void**),
		void ** size_func_params){

		index_item * index = 
			lss_build_index(ls, size_func, size_func_params);
		if(index == NULL){
			ls_error("Could not build index");
			return -1 ;
		}
		lss_store_index(list_wd, index, ls->size);
		free(index);
		return 0;
}

int 
lss_store_list (list * ls,
		const char * list_wd,
		uint32_t (*size_func)(el*, void**),
		void ** size_func_params){
	// -- Sanity checking
	if(ls == NULL || ls->size == 0){
		ls_error("Can't store empty or null list");
		return -1 ;
	}
	if(size_func == NULL){
		ls_error("Invalid size function");
		return -2;
	}
	if(strlen(list_wd) >= PATH_MAX-1){
		ls_error("Path too long to be real");
		return -3 ;
	}
	// ------------- 
	
	// -- This function should only be called when storing new lists,
	// so we create a working directory or spit out an error.
	// ----
	if(!lss_mk_wd(list_wd)){
		return -4;
	}
	// ------------- 
	
	// -- We build and save the index file, which holds the IDs of all the
	// items a list contain, the flag_mask, and the return value of the 
	// user supplied size function for each of the items in the list. Size
	// is deal with this way so that ls doesn't have to deal with the size
	// of the data or the changes thereof.
	// ----
	if(lss_build_save_index(ls, list_wd, size_func, size_func_params)){
		return -5;
	}
	// ------------- 
	
	// -- Now we store each element consecutively (read: foreach-ly), the 
	// files hold nothing but the data. If the element is a list, 
	// lss_store_el calls *this very function* to create a sub-directory
	// list. When we read the list from disk, the existence of the flag_mask
	// in the index file means we needn't use dirent or anything like that.
	// ----
	lss_store_el_callback_data p = {
		ls,
		list_wd,
		size_func,
		size_func_params,
		0 // assume failure
	} ;

	ls_foreach(ls, lss_store_el, (void**) &p);
	// ------------- 
	return 1 ? 0 : -5 ;
}

index_item * lss_load_index(const char * list_wd, unsigned int * items){
	char index_path[PATH_MAX * 2] ;
	sprintf(index_path, "%s/index.lsx", list_wd);
	FILE * index_d = fopen(index_path, "rb") ;
	if(index_d == NULL){
		char err[PATH_MAX * 2];
		perror("File I/O error");
		sprintf(err, "Can't read index file (%s)", index_path);
		ls_error(err);
		return NULL ;
	}

	uint32_t items_32 ; 

	if(fread(&items_32, 4, 1, index_d) != 1){
		perror("File I/O error");
		ls_error("Can't read size of index");
		goto error;
	}

	if(items_32 > SIZE_MAX){
		ls_error("Can't read index on this machine");
		goto error;
	}

	*items = (size_t) items_32 ;

	index_item * index = (index_item*) 
		malloc(*items * sizeof(index_item) ) ;

	if(index == NULL){
		ls_error("Can't load index, out of memory");
		goto error;
	}

	if(*items && fread(index, sizeof(index_item), *items, index_d) 
			!= *items){
		perror("I/O status");
		
		ls_debug_msg("items: %d", *items);	
		ls_error("Cannot read index");
		goto error;
	}

	fclose(index_d);
	return index;  
error:
	fclose(index_d);
	return NULL ;
}

void * lss_fetch_el_data(const char * path, size_t bytes){
	FILE * fp = fopen(path, "rb");

	if(fp == NULL){
		perror("File I/O error");
		char err[PATH_MAX*2] ;
		sprintf(err, "Cannot load (%s)", path);
		ls_error(err);
		return NULL;
	}
	if(!bytes){
		fclose(fp);
		ls_error("Cannot read empty element");
		return NULL;
	}
	
	void * data = (void*) malloc(bytes); // data malloc 
	if(data == NULL){
		ls_error("out of memory");
		return NULL ;

	}
	memset(data, 0, bytes);

	if(fread(data, bytes, 1, fp) != 1 || data == NULL ){
		char err[PATH_MAX+2] ;
		sprintf(err, "Cannot read element (%s) size (%u)", path, bytes);
		ls_error(err);
		free(data);
		return NULL ;
	}

	fclose(fp);
	return data; 

}

list * lss_fetch_list(const char * list_wd){
	list * new = ls_create_empty(0);
	if(new == NULL){
		return NULL;
	}

	unsigned int items = 0;
	index_item * index = lss_load_index(list_wd, &items);
	if(index == NULL){
		ls_error("Can't load index");
		return NULL;
	}

	
	list * sub_list = NULL ;

	char path[PATH_MAX*2] ;
	void * data = NULL ;
	bool adopt_success = 1;
	unsigned int i = 0;
	
	for(; i != items && adopt_success; i++){
		if(index[i].flag_mask & 0x01){ // This is a sub-list
			sprintf(path, "%s/%u", list_wd, index[i].id);
			sub_list = lss_fetch_list(path);
			ls_adopt(new, sub_list, &adopt_success);
		}
		else{ // Plain ol' element
			sprintf(path, "%s/%u.ls", list_wd, index[i].id);
			data = lss_fetch_el_data(path, index[i].size_32);
			ls_append(new, data, index[i].size_32);
			if(data != NULL) free(data);
		}
	}
	
	free(index);
	return new;
}

