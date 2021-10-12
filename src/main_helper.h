#ifndef MAIN_HELPER_H
#define MAIN_HELPER_H


// =================== //
// === PRINT ERROR === //
// =================== //

void print_failed_to_build() {
	fprintf(stderr, "Failed to generate build commands!\n");
}


void print_cyclic_dependency() {
	fprintf(stderr, "Cyclic module dependency detected. ");
	print_failed_to_build();
}

void print_invalid_module_syntax(const char* filename, unsigned long line_num) {
	fprintf(stderr, "Error encountered in file %s:\n", filename);
	fprintf(stderr, "Invalid module syntax at line %lu. ", line_num);
	print_failed_to_build();
}

void print_error(const char* err)
{
	fprintf(stderr, "%s. ", err);
	print_failed_to_build();
}

void print_error_no_file(const char* filename)
{
	fprintf(stderr, "Could not read File '%s'. ", filename);
	print_failed_to_build();
}


// =================== //
// === PRINT USAGE === //
// =================== //

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
	printf("--compiler=g++-11 ");
	printf("--compile-flags='' ");
	printf("--file=build_output.txt ");
	printf("--output=premake5");
	printf("\n");
	printf("Options:\n");
	printf("  --compiler=VALUE           VALUE is name of compiler executable\n");
	printf("  --compiler-flags='PATTERN' PATTERN is flags for the compiler\n");
	printf("  --file=FILE                if this flag is specified, output build\n");
	printf("                             commands to FILE insted of standard output\n");
	printf("  --output=FORMAT            output is formatted to match specific build\n");
	printf("                             tools, where FORMAT may be one of the following:\n");
	printf("                             premake5, terminal\n");
	printf("  --debug-print              print debug information to terminal\n");
	printf("\n");
	printf("Report bugs by opening an issue on github:\n");
	printf("<https://github.com/alexpanter/cpp_module_parser/issues>\n");
}


#endif // MAIN_HELPER_H
