#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "program_args.h"
#include "io_reader.h"
#include "module_unit.h"
#include "main_helper.h"

#define CMOP_MAX_LINE_SIZE 1024


int main(int argc, char* argv[])
{
	// TODO: This might be implemented through I/O-redirection,
	// though does that work on other platforms than Linux?

	if (argc < 2) {
		print_usage();
		return 2;
	}
	if (argc == 2 && (strcmp(argv[1], "--help") == 0)) {
		print_help();
		return 0;
	}
	printf("argv[1]: \"%s\"\n", argv[1]);

	// read command-line args
	program_args_t args;
	program_args_init(&args);
	program_args_parse(argc, argv, &args);
	program_args_print(&args);

	// BUILD ORDER:
	// 1) build headers
	// 2) build module dependency graph
	// 3) build non-module translation units

	char* filename = malloc(sizeof(unsigned char) * CMOP_MAX_LINE_SIZE);
	while(scanf("%1023s[^\n]", filename) == 1)
	{
		printf("reading file: %s\n", filename);
		module_unit_t unit;
		module_unit_init(&unit);

		read_status_t status = read_file(filename, &unit);
		if (status == READ_STATUS_FILE_NOT_EXISTS)
			printf("--> %s\n", get_read_status_string(status));

		//module_unit_debug_print(&unit);
		module_unit_free(&unit);
	}
	free(filename);

	return 0;
}
