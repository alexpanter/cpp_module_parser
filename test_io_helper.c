#include "io_helper.h"
#include <stdio.h>

#define TEST_PRINT_ALL
#define CHECK_NULL(str) ((str == NULL) ? "<null>" : str)

void testline(const char* line)
{
	module_lineinfo_t info;
	init_lineinfo(&info);
	read_line(strdup(line), &info);
#if defined(TEST_PRINT_ALL)
	printf("\"%s\"\n", line);
	printf("\tlinetype: %s\n", get_linetype_string(info.linetype));
	printf("\tmodule-name: \"%s\"\n", info.module_name);
	printf("\tpartition-name: \"%s\"\n", info.module_partition_name);
	printf("\theader-name: \"%s\"\n", info.header_name);
#endif
	free_lineinfo(&info);
}

void testfile(char* filename)
{
	module_unit_t unit;
	module_unit_init(&unit);

	printf("testfile: %s\n", filename);
	io_read_status_t status = parse_file(filename, &unit);
	printf("--> io_read_status: %s\n", get_io_status_string(status));
	printf("--> module-type: %s\n", get_module_type_string(unit.module_type));
	printf("--> module-name: %s\n", CHECK_NULL(unit.module_name));
	printf("--> module-partition-name: %s\n", CHECK_NULL(unit.module_partition_name));

	module_unit_free(&unit);
	printf("\n");
}


int main()
{
	/*
	  testline("module;");
	  testline("module ;");
	  testline("export module banana;");
	  testline("export module banana ;");
	  testline("import mango;");
	  testline("import mango ;");
	  testline("export import mango");
	  testline("export module orange:blood_orange;");
	  testline("export module orange :blood_orange;");
	  testline("export import :blood_orange");
	*/
	testfile("example_files/ignore.cpp");
	testfile("example_files/is_module_test/negative1.cpp");
	testfile("example_files/is_module_test/negative2.cpp");

	return 0;
}
