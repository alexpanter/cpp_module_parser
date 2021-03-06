#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "program_args.h"
#include "io_reader.h"
#include "module_unit.h"
#include "main_helper.h"
#include "string_helper.h"

#define CMOP_MAX_LINE_SIZE 1024


int main(int argc, char* argv[])
{
	// TODO: Input files are read through I/O-redirection,
	// though does that work on other platforms than Linux?

	if (argc < 2) {
		print_usage();
		return 2;
	}
	if (argc == 2 && (strcmp(argv[1], "--help") == 0)) {
		print_help();
		return 0;
	}

	// read command-line args
	program_args_t args;
	program_args_init(&args);
	int parse_err = program_args_parse(argc, argv, &args);
	if (parse_err) {
		program_args_free(&args);
		return 1;
	}
	if (args.debug_print) program_args_print(&args);

	// BUILD ORDER:
	// 1) parse files provided in stdin
	// 2) build module dependency graph
	// 3) generate build order
	// 4) output build order
	// 5) drink coffee and smile :)

	// Read through files one at a time
	char* filename = malloc(sizeof(unsigned char) * CMOP_MAX_LINE_SIZE);
	while(scanf("%1023s[^\n]", filename) == 1)
	{
		if (strcmp(filename, "done;") == 0) break;

		module_unit_t unit;
		module_unit_init(&unit);

		read_status_t status = read_file(filename, &unit);
		if (status == READ_STATUS_FILE_NOT_EXISTS) {
			print_error_no_file(filename);
			module_unit_free(&unit);
			free(filename);
			program_args_free(&args);
			return 1;
		}
		else if (status == READ_STATUS_INVALID_SYNTAX) {
			print_invalid_module_syntax(filename, unit.line_num);
			module_unit_free(&unit);
			free(filename);
			program_args_free(&args);
			return 1;
		}

		// File exists, and it was parsed.
		// If file is a module unit place it in the dependency graph.
		// An argument can be made against this, since one of the next files
		// may fail the syntax checker, and updating the dependency graph is
		// likely an expensive operation.

		if (args.debug_print) module_unit_debug_print(&unit);
		module_unit_free(&unit);
	}

	// Parse the dependency graph

	// Generate build commands

	// Cleanup
	free(filename);
	program_args_free(&args);

	return 0;
}
