#include "libls.h"
#include "ls_print.h"
#include <stdio.h>

#define TAB(level)	(printf("%*c", level, ' '))

void ls_print_tree(list * ls){
	printf(" --{ %s }-- \n"
               "       |\n", ls == NULL ? "EMPTY" : "MAIN" );
	ls_foreach(ls, ls_print_el, (void**) ls);
	printf("\n");
}

bool ls_print_el(el * i, void ** arg){
	if(ls_is_element_sub(i) && arg != NULL){
		TAB(6); 
		ls_print_sub_el(i, arg) ;
	}
	else {
		TAB(6);
		printf("[ ID(%d) int(%d) ]\n", i->id, ls_get_from(i, int) );
	}
	return 1;
}

bool ls_print_sub_el(el * i, void **arg){
	list * mother = ((list*) arg);
	list * sub_list = ls_get_sublist(mother, i->id);

	TAB(6);
	printf("{ ID(%d) SIZE(%d) } -> ", i->id, sub_list->size );

	if(sub_list->size == 0){
		printf("(EMPTY)\n");
	}
	else {
		ls_foreach(sub_list, ls_print_el, arg);
	}
	return 1;
}
