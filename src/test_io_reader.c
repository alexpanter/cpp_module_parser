#include "io_reader.h"
#include <stdio.h>

void testfile(char* filename)
{
	module_unit_t unit;
	module_unit_init(&unit);
	read_status_t status = read_file(filename, &unit);

	printf("PARSE_FILE(%s):\n", filename);
	printf("--> read_status: %s\n", get_read_status_string(status));

	switch (status)
	{
	case READ_STATUS_FILE_NOT_EXISTS:
		break;
	case READ_STATUS_INVALID_SYNTAX:
		printf("--> Error found at line %li\n", unit.line_num);
		break;
	case READ_STATUS_MODULE:
		module_unit_debug_print(&unit);
		break;
	case READ_STATUS_NOT_MODULE:
	default:
		break;
	}

	module_unit_free(&unit);
	printf("\n");
}

int main()
{
	//testfile("example_files/non_existent_file.cpp");
	testfile("example_files/ignore.cpp");

	return 0;
}
