#include<stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#define LS_DEBUG 

// #define ls_get_element_sub(element)		ls_get_from(
#define	ls_get_from(element, type)	*((type*)(element->data))
#define	ls_get(ls, id, type)		*((type*)(ls_get_el(ls, id)->data))
#define ls_get_p(ls, id, type)		((type)(ls_get_el(ls, id)->data))
#define ls_get_from_p(element, type)	((type)(element->data)) 

// First bit: Indicates whether this element is a sub-tree or not
#define ls_is_element_sub(element)	(element->		flag_mask & 0x1)
#define ls_is_sub(ls, id)		(ls_get_el(ls, id)->	flag_mask & 0x1)
#define ls_is_sub_list(ls)		(ls->			flag_mask & 0x1)

typedef struct el {
	unsigned int id ;
	void * data ;

	int8_t flag_mask ;
	
	struct el * next ; 
	struct el * prev ;
} el;


typedef struct list {
	unsigned int size ; // number of elements allocated
	unsigned int max_size ; // maximum number of elements allowed in the
				// list.
		       // 0 for infinte
	int8_t flag_mask ; // reserved 

	el * head ; // first element in the list
	el * tail ; // last element in the list
	
	struct list * mother ; // mother list, NULL for none.
} list ;

list * ls_create_empty(int max_size);
list * ls_create(void ** data, unsigned int elements, size_t size,
		unsigned int max_size);

list * ls_get_sublist(list * mother, int id) ; 

int ls_append(list * ls, void * data, size_t size);
int ls_append_empty_sub(list * mother, int sub_max_size);

el * ls_get_el(list * ls, unsigned int id) ; 
unsigned int ls_adopt(list * mother, list * sub, bool * success);

unsigned int  ls_foreach(list * ls, bool (*func)(el*, void**), void ** args);
void ls_foreach_sub(list * ls, bool (*func)(el*, void**), void ** args);

//void ls_eachfor(list * ls, void (*func)(el*, void**), void ** args); 

void ls_set_error(void (*callback)(char*));
void ls_error_standard(char * msg);

void ls_purge(list * ls);

void ls_free_list(list * ls);
