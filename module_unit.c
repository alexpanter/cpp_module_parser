#include <stdlib.h>
#include <stdio.h>
#include "module_unit.h"

#define CHECK_NULL_STR(str) ((str == NULL) ? "<null>" : str)


void module_unit_init(module_unit_t* unit)
{
	unit->module_type = MODULE_TYPE_UNDETERMINED;
	unit->filename = NULL;
	unit->module_name = NULL;
	unit->partition_name = NULL;
	unit->module_deplist = NULL;
	unit->partition_deplist = NULL;
	unit->header_deplist = NULL;
}

void module_unit_free(module_unit_t* unit)
{
	if (unit->filename != NULL) free(unit->filename);
	if (unit->module_name != NULL) free(unit->module_name);
	if (unit->partition_name != NULL) free(unit->partition_name);
	unit->filename = NULL;
	unit->module_name = NULL;
	unit->partition_name = NULL;

	module_unit_deplist_t* ptr = unit->module_deplist;
	while (ptr != NULL)
	{
		module_unit_deplist_t* next = ptr->next;
		free(ptr->name);
		free(ptr);
		ptr = next;
	}

	ptr = unit->partition_deplist;
	while (ptr != NULL)
	{
		module_unit_deplist_t* next = ptr->next;
		free(ptr->name);
		free(ptr);
		ptr = next;
	}

	ptr = unit->header_deplist;
	while (ptr != NULL)
	{
		module_unit_deplist_t* next = ptr->next;
		free(ptr->name);
		free(ptr);
		ptr = next;
	}

	unit->module_deplist = NULL;
	unit->partition_deplist = NULL;
	unit->header_deplist = NULL;
}


void module_unit_addimport_module(module_unit_t* unit, char* name)
{
	module_unit_deplist_t* dep = malloc(sizeof(module_unit_deplist_t));
	dep->name = name;
	dep->next = unit->module_deplist;
	unit->module_deplist = dep;
}

void module_unit_addimport_partition(module_unit_t* unit, char* name)
{
	module_unit_deplist_t* dep = malloc(sizeof(module_unit_deplist_t));
	dep->name = name;
	dep->next = unit->partition_deplist;
	unit->partition_deplist = dep;
}

void module_unit_addimport_header(module_unit_t* unit, char* name)
{
	module_unit_deplist_t* dep = malloc(sizeof(module_unit_deplist_t));
	dep->name = name;
	dep->next = unit->header_deplist;
	unit->header_deplist = dep;
}


void module_unit_print_deplist(module_unit_t* unit)
{
	module_unit_deplist_t* ptr = unit->module_deplist;
	while (ptr != NULL)
	{
		printf("----> [MODULE] \"%s\"\n", ptr->name);
		ptr = ptr->next;
	}

	ptr = unit->partition_deplist;
	while (ptr != NULL)
	{
		printf("----> [PARTITION] \"%s\"\n", ptr->name);
		ptr = ptr->next;
	}

	ptr = unit->header_deplist;
	while (ptr != NULL)
	{
		printf("----> [HEADER] \"%s\"\n", ptr->name);
		ptr = ptr->next;
	}
}


void module_unit_debug_print(module_unit_t* unit)
{
	printf("--> filename: %s\n", CHECK_NULL_STR(unit->filename));
	printf("--> module-type: %s\n",
		   get_module_type_string(unit->module_type));
	printf("--> module-name: \"%s\"\n",
		   CHECK_NULL_STR(unit->module_name));
	printf("--> partition-name: \"%s\"\n",
		   CHECK_NULL_STR(unit->partition_name));
	printf("--> dependencies:\n");
	module_unit_print_deplist(unit);
}


const char* get_module_type_string(module_type_t mt)
{
	switch (mt)
	{
	case MODULE_TYPE_UNDETERMINED: return "MODULE_TYPE_UNDETERMINED";
	case MODULE_TYPE_NONE:         return "MODULE_TYPE_NONE";
	case MODULE_TYPE_INVALID:      return "MODULE_TYPE_INVALID";
	case MODULE_TYPE_MODULE:       return "MODULE_TYPE_MODULE";
	case MODULE_TYPE_PARTITION:    return "MODULE_TYPE_PARTITION";
	default:
		return "ERROR: Invalid module_type\n";
	}
}

