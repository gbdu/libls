
#ifdef LS_DEBUG 
void ls_debug_el(el * i){
	printf("\n------ ls_debug_el ------------\n");
	if(i == NULL){
		printf("i is null!\n");
		return ;
	}
	printf( "addr: %p\n"
		"id: %d\n"
		"data: %p\n"
		"flag_mask: %x\n"
		"next: %p\n"
		"prev: %p\n",
		i, i->id, i->data, i->flag_mask, i->next, i->prev);
	printf("------ End of ls_debug_el ------- \n");
}
void ls_debug_el(el * i){

void ls_debug_list(list * ls){
	printf("\n -------- ls_debug_list --------\n");
	if(ls == NULL){
		printf("ls is null!");
	}
	printf( "addr: %p\n" 
		"size: %d\n"
		"max_size: %d\n"
		"flag_mask: %x\n"
		"head: %p\n"
		"tail: %p\n"
		"mother: %p\n",
		ls, ls->size, ls->max_size, ls->flag_mask, ls->head, ls->tail,
		ls->mother);
	printf("------- end of ls_debug_list ------\n");
}
#define ls_debug_msg(msg, args)		printf("\t ls_debug_msg:  " msg, args); 
#define ls_deb(msg)			printf("-----{ls_debug: %s}-----\n", msg);
#endif

