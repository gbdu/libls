void ls_debug_el(el * i);

#define ls_debug_msg(msg, args)		printf("\t ls_debug_msg:  " msg, args); 
#define ls_deb(msg)			printf("-----{ls_debug: %s}-----\n", msg);

void ls_debug_list(list * ls);
