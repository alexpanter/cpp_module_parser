#ifndef MODULE_UNIT_H
#define MODULE_UNIT_H

#include "dependency_list.h"

typedef enum fefe
{
	MODULE_TYPE_UNDETERMINED,
	MODULE_TYPE_NONE,
	MODULE_TYPE_INVALID,
	MODULE_TYPE_MODULE,
	MODULE_TYPE_PARTITION
} module_type_t;

typedef struct module_unit_t
{
	dependency_list_t* dependency_list;
	char* module_name;
	char* module_partition_name;
	module_type_t module_type;
} module_unit_t;


/*
 * Initialize module info.
 */
void module_unit_init(module_unit_t* info);


/*
 * Free module info.
 */
void module_unit_free(module_unit_t* info);

void module_unit_printdeps(module_unit_t* info);


const char* get_module_type_string(module_type_t mt);



#endif // MODULE_UNIT_H
