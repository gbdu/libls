#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "libls.h"
#include "ls_print.h"
#include "ls_store.h"

typedef struct post {
	char topic[255];
	char content[255];
	int time ;
	int user_id ;
} post ;

uint32_t newlist_el_size(el * el, void ** args){
	return sizeof(post);
}

bool print_post(el * e, void ** p){
	post * ip = ls_get_from_p(e, post*); 
	printf("User: %d\n", ip->user_id );
	printf("Content: %s\n\n", ip->content);
	return 1;
}

int main(void){
	list * newlist = ls_create_empty(0);
	unsigned int subid = 0;
	int c = 0;

	post * p = (post*) malloc(sizeof(post));
	p->user_id = 15;
	p->time = 10;
	strcpy(p->topic, "hello");
	strcpy(p->content, "world");
	
	for(; c != 10; c++){
		ls_append(newlist, p, sizeof(post)) ;
		//subid = ls_append_empty_sub(newlist, 10);
		//	ls_append(ls_get_sublist(newlist, subid), p, sizeof(p));
	}
	
	free(p);

	ls_foreach(newlist, print_post, NULL);
	
	//ls_print_tree(newlist);

	//lss_store_list(newlist, "newdir", newlist_el_size, NULL);
	
	//lss_build_save_index(newlist, "newdir", newlist_el_size, NULL);
	
	//ls_purge(newlist);	

	//newlist = lss_fetch_list("newdir");
	
	//ls_print_tree(newlist);
	
	//ls_purge(newlist);
	return 0;
}
