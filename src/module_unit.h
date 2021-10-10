#ifndef MODULE_UNIT_H
#define MODULE_UNIT_H


typedef enum
{
	MODULE_TYPE_UNDETERMINED,
	MODULE_TYPE_NONE,
	MODULE_TYPE_INVALID,
	MODULE_TYPE_MODULE,
	MODULE_TYPE_PARTITION
} module_type_t;

typedef struct module_unit_deplist_t
{
	char* name;
	struct module_unit_deplist_t* next;
} module_unit_deplist_t;

typedef struct module_unit_t
{
	char* filename; // TODO: Use absolute file path instead?
	char* module_name;
	char* partition_name;
	module_type_t module_type;
	module_unit_deplist_t* module_deplist;
	module_unit_deplist_t* partition_deplist;
	module_unit_deplist_t* header_deplist;
	unsigned long line_num; // Used for reporting errors
} module_unit_t;


/*
 * Initialize module info.
 */
void module_unit_init(module_unit_t* unit);


/*
 * Free module info and release its resources.
 */
void module_unit_free(module_unit_t* unit);

/*
 * Add module dependencies.
 */
void module_unit_addimport_module(module_unit_t* unit, char* name);
void module_unit_addimport_partition(module_unit_t* unit, char* name);
void module_unit_addimport_header(module_unit_t* unit, char* name);


/*
 * Check module dependencies.
 */
int module_unit_imports_module(module_unit_t* unit, char* name);
int module_unit_imports_partition(module_unit_t* unit, char* name);
int module_unit_imports_header(module_unit_t* unit, char* name);


/*
 * String representation of `module_type_t`, useful
 * for print debugging.
 */
const char* get_module_type_string(module_type_t mt);

/*
 * Print module dependencies, useful for print debugging.
 */
void module_unit_print_deplist(module_unit_t* unit);

/*
 * Print all captured information about the module unit.
 */
void module_unit_debug_print(module_unit_t* unit);



#endif // MODULE_UNIT_H
