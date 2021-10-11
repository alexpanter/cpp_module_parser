#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "program_args.h"
#include "io_reader.h"
#include "module_unit.h"

#define APPNAME "cmop"
#define CMOP_MAX_LINE_SIZE 1024

const char* cyclic_dependency() {
	return "Cyclic module dependency detected. Aborting build script!";
}

const char* invalid_module_syntax() {
	return "Invalid module syntax. Failed to generate build script!";
}

void print_error(const char* err)
{
	fprintf(stderr, "%s. Failed to generate build script!\n", err);
}

void print_usage_pattern()
{
	printf("Usage: %s [OPTION]... PATTERNS\n", APPNAME);
}

void print_usage()
{
	print_usage_pattern();
	printf("Try '%s --help' for more information.\n", APPNAME);
}

void print_help()
{
	print_usage_pattern();
	printf("Generate build scripts for C++20 projects that use modules.\n");
	printf("Example: %s ", APPNAME);
	printf("--files='src/**/*.cpp' ");
	printf("--compiler='g++-11' ");
	printf("--compiler-flags='' ");
	printf("--output='stdout' ");
	printf("\n");
	printf("Options:\n");
	printf("  --files='PATTERN'          include files matching PATTERN\n");
	printf("  --compiler=VALUE           VALUE is name of compiler executable\n");
	printf("  --compiler-flags='PATTERN' PATTERN is flags for the compiler\n");
	printf("  --output=FILE              if this flag is specified, output build\n");
	printf("                             commands to FILE insted of standard output\n");
	printf("\n");
	printf("Report bugs by opening an issue on github:\n");
	printf("<https://github.com/alexpanter/cpp_module_parser/issues>\n");
}

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

	// BUILD ORDER:
	// 1) build headers
	// 2) build module dependency graph
	// 3) build non-module translation units

	char* filename = malloc(sizeof(unsigned char) * CMOP_MAX_LINE_SIZE);
	//while (fgets(filename, CMOP_MAX_LINE_SIZE-1, stdin))
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
