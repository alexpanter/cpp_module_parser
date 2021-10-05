#include "io_helper.h"
#include <stdio.h>
#include <stdlib.h>

#define TEST_PRINT_ALL

void testline(const char* line)
{
	module_lineinfo_t info;
	init_lineinfo(&info);
	char* new_line = strdup(line);
	read_line(new_line, &info);
#if defined(TEST_PRINT_ALL)
	printf("\"%s\"\n", line);
	printf("\tlinetype: %s\n", get_linetype_string(info.linetype));
	printf("\tmodule-name: \"%s\"\n", info.module_name);
	printf("\tpartition-name: \"%s\"\n", info.partition_name);
	printf("\theader-name: \"%s\"\n", info.header_name);
#endif
	free_lineinfo(&info);
	free(new_line);
}

void testfile(char* filename)
{
	module_unit_t unit;
	module_unit_init(&unit);
	io_read_status_t status = parse_file(filename, &unit);

	printf("PARSE_FILE(%s):\n", filename);
	printf("--> io_read_status: %s\n", get_io_status_string(status));

	if (status == IO_READ_STATUS_MODULE ||
		status == IO_READ_STATUS_MODULE_PARTITION)
	{
		module_unit_debug_print(&unit);
	}

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

	testfile("example_files/non_existent_file.cpp");
	testfile("example_files/ignore.cpp");
	testfile("example_files/export_module_computer.cpp");
	testfile("example_files/export_module_computer-cpu.cpp");
	testfile("example_files/large_module_unit.cpp");

	testfile("example_files/is_module_test/negative1.cpp");
	testfile("example_files/is_module_test/negative2.cpp");

	return 0;
}
