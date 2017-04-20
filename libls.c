#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "libls.h"
#include "ls_deb.h"

void ls_error_standard(char * msg){
	printf("Fatal ls error: %s.\n", msg);
	exit(1);
}

void (*ls_error)(char*) = ls_error_standard;

void ls_set_error(void (*callback)(char*)){
	ls_error = callback;
}

int ls_rem(list * ls, unsigned int id) ; 

bool ls_el_remove(el * i, void ** arg) ;

int ls_append(list * ls, void * data, size_t size){
	el * new = (el*) malloc(sizeof(el));
	if(new == NULL){
		ls_error("out of memory");
	}
	// We fill in this order:
	// Data - ID - Prev - Next - Mask (zero'd out for default)
	// -- Populate the data
	if(data != NULL){
		new->data = (void*) malloc(size) ;
		if(new->data == NULL){
			ls_error("out of memory");
			return -1;
		}
		memcpy(new->data, data, size) ;
	} else {
		ls_error("elements cannot point to NULL values");
		return -1;
	}
	// ------------- 
	
	// -- Actually add it to the list, it will short-circuit 
	// on infinite+empty
	if(ls->max_size && ls->size == ls->max_size){ 
		ls_error("list is full, cannot append");
		return -1 ;
	} // else ...

	ls->size++;
	
	new->id = ls->tail == NULL ? 0 : ls->tail->id + 1; // First element?

	new->next = NULL ; // Nothing to follow.
	new->prev = ls->tail == NULL ? NULL : ls->tail ; // Nothing to come.
	new->flag_mask = 0x0; // Zero'd for default

	if(ls->tail == NULL && ls->head == NULL){ // The list was empty
		ls->head = new ;
		ls->tail = new ;
	} else{
		ls->tail->next = new ; // This won't be the tail anymore,
		ls->tail = new ; // This will.
	}
	// ------------- 

	return new->id ; 
}

unsigned int ls_append_el(list * ls, el * element, size_t data_size,
		bool preserve_id, bool preserve_flag_mask){
	unsigned int new_id = ls_append(ls, element->data, data_size);
	
	if(preserve_id){
		// TODO: what if the file id already exists?
		new_id = ls_get_el(ls, new_id)->id = element->id;
	}
	if(preserve_flag_mask){
		//ls_get_el(ls, new_id)-> 
	}

	return new_id ; 
}

int ls_append_empty_sub(list * mother, int sub_max_size){
	list * sub = ls_create_empty(sub_max_size);
	if(sub == NULL){
		return -1;
	}

	sub->flag_mask = 0x1 ; // Bit one: true for subtree
	sub->mother = mother;

	// -- IMPORTANT: The pointer to the sub-list will be held in
	// the data field of the node in the mother. (what?)
	void * data = &sub ;
	int sub_id = ls_append(mother, data, sizeof(data)) ;
	// printf("r: %p\n", (ls_get(mother, sub_id, list*))) ;
	// ----

	// -- First bit of the flag mask is true for subtrees, so we get
	// the pointer and we set the flag_mask appropriately.
	ls_get_el(mother, sub_id)->flag_mask = 0x1 ;
	// ----
	
	return sub_id ; 
}

unsigned int ls_adopt(list * mother, list * sub, bool * success){
	if(mother == NULL || sub == NULL){
		ls_error("NULL mother or sub pointer");
		if(success != NULL){
			*success = 0;
		}
	}
	
	void * data = &sub ;
	unsigned int sub_id = ls_append(mother, data, sizeof(data)) ;
	
	ls_get_el(mother, sub_id)->flag_mask = 0x1 ;
	sub->flag_mask = sub->flag_mask | 0x01 ;
	
	if(success != NULL){
		*success = 1;
	}

	return sub_id ;
}


list * ls_get_sublist(list * mother, int sub_id){
	list * ret = ls_get(mother, sub_id, list*);
	return ret ;
}

list * ls_create_empty(int max_size){
	list * new = (list*) malloc(sizeof(list));
	if(new == NULL){
		ls_error("out of memory");
		return NULL ;
	}

	new->size = 0;
	new->max_size = max_size;
	new->head = NULL ;
	new->tail = NULL ;
	new->flag_mask = 0x0 ; // not a subtree
	new->mother = NULL ;
	return new;
}

list * ls_create(void ** data, unsigned int elements, size_t size,
		unsigned int max_size){
	list * new = ls_create_empty(max_size) ;
	if(new == NULL){
		ls_error("cannot create empty list");
		return NULL;
	}
	else if(elements == 0){ // this is an empty list
		return new ; 
	}
	else if(elements > max_size){
		ls_error("number of elements is bigger than max_size");
		return NULL;
	} // -- no errors so far, proceed as normal
	else if(elements == max_size){ // this is a fixed sized list
		new->size = 0 ; // temporarly set the size to infinite
	}
	else {
		new->size = elements ;
	}

	// -- in case we have a fixed-size list with a number of elements
	// supplied, we set it as infinite temporarly so ls_append won't choke
	// up
	new->max_size = elements == max_size ? 0 : max_size;
	// ------------- 
	
	int c = 0;
	for(; c != elements; c++){
		if(ls_append(new, (void*) (data)[c], size) < 0){
			return NULL ;
		}
	}
	
	// -- if it is infact a fixed sized list, set the max_size back as
	// the user expects.
	if(elements == max_size){
		new->max_size = max_size ;
	}
	// ------------- 

	return new;
}

unsigned int ls_foreach(list * ls, bool (*func)(el*, void**), void ** args) {
	if(ls == NULL || (ls->head == NULL && ls->tail == NULL) ) {
		// todo: add warning call back here
		ls_error("foreach on a NULL/empty list");
		return 0 ;
	}
	if(ls->size == 0){
		return 0;
	}
	
	el * index_element = ls->head ; // start from head
	el * index_next = NULL;
	
	// -- note: we copy the index so if the func() alters the value of
	// next/prev, we've got nothing to worry about. (for exmaple,
	// if func() removed the element)
	
	if(ls->head == ls->tail){
		// this is a one-element list, call func and bail out
		func(index_element, args) ; // or tail
		return 0 ;	
	}

	while(index_element != NULL){
		index_next = index_element->next; 
		if(!func(index_element, args)){
			return index_element->id ;
		}
		index_element = index_next; // go forward
	}
	// ------------- 
	
	return 0 ;
}

typedef struct ls__foreach_sub_callback_data {
	bool (*func)(el *, void**) ;
	void ** args;
} ls__foreach_sub_callback_data;

bool ls__foreach_sub_callback(el * e, void **args){
	ls__foreach_sub_callback_data * d = (ls__foreach_sub_callback_data*)
		args;

	bool (*user_func)(el *, void**) = d->func;
	void ** user_args = d->args;

	if(ls_is_element_sub(e)){
		user_func(e, user_args);
	}

	return 1; 

}

void ls_foreach_sub(list * ls, bool (*func)(el*, void**), void ** args){
	ls__foreach_sub_callback_data p = {
		func,
		args
	} ;

	ls_foreach(ls, ls__foreach_sub_callback, (void*) &p);
}

typedef struct ls_is_el_callback_data {
	unsigned int wanted_id ;
	el * found ; 
} ls_is_el_callback_data ;

bool ls_is_el(el * e, void ** vargs){
	ls_is_el_callback_data * p = (ls_is_el_callback_data*) vargs ;
	if(e->id == p->wanted_id) {
		p->found = e ;
		return 0 ; // stop
	}
	return 1; // continue 
}

el * ls_get_el(list * ls, unsigned int id){
	ls_is_el_callback_data p = { id, NULL } ;
	ls_foreach(ls, ls_is_el, (void**) &p) ;
	
	if(p.found == NULL){
		char err[32];
		sprintf(err, "invalid index: [%d]", p.wanted_id);
		ls_error(err);
		return NULL ;
	}

	return p.found ; 
}

int ls_rem(list * ls, unsigned int id){
	ls_el_remove(ls_get_el(ls, id), (void**) ls);
	return 0 ;
}

int ls_chop_branch(list * ls, list * from, unsigned int id){
	return 1 ;	
}

bool ls_el_disown(el * i, void ** arg_list){
	list * ls = (list*) arg_list;
	
	if(i == NULL || ls == NULL){
		ls_error("invalid disown");
		return 0;
	}
	if(ls_is_element_sub(i)) {
		ls_deb("you're not supposed to get here");
		return 0;
	}
	if(i->next != NULL && i->prev != NULL){
		// we're somewhere in the middle
		i->next->prev = i->prev ;
		i->prev->next = i->next ;
	}
	else if(i->next == NULL ){ // this is the tail	
		if(i->prev != NULL){
			ls_deb("not good 1 ");
			i->prev->next = NULL ; // make this the tail
			ls->tail = i->prev ;
		}
	}
	else if(i->prev == NULL ){ // this is the head
		if(i->next != NULL){
			i->next->prev = NULL ;
			ls->head = i->next;
		}
	}
	else { // this is a one-man list (next == prev == nul)
		return 1;
	}

	if(ls->size != 0) ls->size-- ;
	return 1; 
}

el * ls_disown_el(list * list, unsigned int id){
	el * element = ls_get_el(list, id);
	ls_el_disown(element, (void**) list);
	return element ;
}

list * ls_disown_list(list * mother_list, unsigned int id){
	list * sub_list = ls_get_sublist(mother_list, id);
	el * sub_list_element = ls_get_el(mother_list, id);

	ls_el_disown(sub_list_element, (void**) mother_list);

	return sub_list ;
}

bool ls_el_remove(el * i, void ** mother_arg){
	list * mother = (list*) mother_arg;
	
	if(!ls_is_element_sub(i)){ // this is an elements
		ls_el_disown(i, mother_arg);
		if(i->data != NULL){
			free(i->data);
		} else {
			ls_deb("data is null");
		}
		free(i);
		return 1;
	}
	else {
		// does recursion+callback do you head in?
		list * sub_list = ls_get_sublist(mother, i->id);
		if(sub_list->size){
			ls_foreach(sub_list, ls_el_remove, (void**) sub_list);
		}
		i->flag_mask = 0x0 ; // no longer a sub list really
		
		free(sub_list);
		ls_el_remove(i, (void**) mother);
		return 1 ;  
	}
}

void ls_purge(list * ls){
	ls_foreach(ls, ls_el_remove, (void**) ls);
	free(ls);
	return  ;
}

void ls_free_list(list * ls){
	if(ls != NULL){
		free (ls);
	}
}
