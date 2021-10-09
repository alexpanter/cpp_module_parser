#include "dependency_list.h"
#include "module_unit_info.h"


typedef struct dependency_t
{
	module_unit_info_t* module_unit_info;
	struct dependency_t* next;
} dependency_t;

typedef struct dep_list_impl
{
	dependency_t* list;
	unsigned int count;
} dep_list_impl;
