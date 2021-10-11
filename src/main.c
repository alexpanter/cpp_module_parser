#include <stdio.h>
#include <string.h>
#include "program_args.h"

#define APPNAME "cmop"
#include "main_helper.h"



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
	program_args_parse(argc, argv, args);

	// BUILD ORDER:
	// 1) build headers
	// 2) build module dependency graph
	// 3) build non-module translation units







	return 0;
}
