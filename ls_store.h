
// index_item * lss_get_index(const char * list_wd, unsigned int * items);
// int lss_store_index(const char * list_wd, index_item * index, size_t items);


typedef struct index_item { 
	uint32_t id ;
	uint32_t size_32 ;
	int8_t flag_mask ;
} index_item ;

int 
lss_store_list (list * ls,
		const char * list_wd,
		uint32_t (*size_func)(el*, void**),
		void ** size_func_params);

list * lss_fetch_list(const char * list_wd);

