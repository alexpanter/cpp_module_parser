#include "program_args.h"
#include "string_helper.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


int program_args_parse(int argc, char* argv[], program_args_t* args)
{
    char arg[256];
    arg[0] = '\0';
    for (int i = 1; i < argc; i++)
    {
        if (sscanf(argv[i], "--compiler=%255s", arg) == 1) {
            if (args->compiler != NULL) free(args->compiler);
            args->compiler = strdup(arg);
        }
        else if (sscanf(argv[i], "--compiler-flags=%255s", arg) == 1) {
            if (args->compiler_flags != NULL) free(args->compiler_flags);
            args->compiler_flags = strdup(arg);
        }
		// output to a specific file instead of stdout
        else if (sscanf(argv[i], "--file=%255s", arg) == 1) {
            if (args->file != NULL) free(args->file);
            args->file = strdup(arg);
        }
		else if (sscanf(argv[i], "--output=%255s", arg) == 1) {
			if (strcmp(arg, "premake5") == 0) {
				args->output_format = OUTPUT_FORMAT_PREMAKE_5;
			}
			else if (strcmp(arg, "terminal") == 0) {
				args->output_format = OUTPUT_FORMAT_TERMINAL;
			}
			else {
				printf("Invalid value for --output option. ");
				printf("Use the option \"--help\" for a list of commands.\n");
				return 1;
			}
        }
		else if (strcmp(argv[i], "--debug-print") == 0) {
			args->debug_print = 1;
		}
        else {
            printf("Invalid command-line argument: \"%s\".\n", argv[i]);
            printf("Use the option \"--help\" for a list of commands.\n");
			return 1;
        }
    }
    return 0;
}

void program_args_init(program_args_t* args)
{
    args->compiler = NULL;
    args->compiler_flags = NULL;
    args->file = NULL;
	args->output_format = OUTPUT_FORMAT_TERMINAL; // default
	args->debug_print = 0;
}

void program_args_free(program_args_t* args)
{
	if (args->compiler != NULL) free(args->compiler);
    if (args->compiler_flags != NULL) free (args->compiler_flags);
    if (args->file != NULL) free(args->file);

    program_args_init(args);
}

const char* getstr_output_format(output_format_t format)
{
	switch (format)
	{
	case OUTPUT_FORMAT_TERMINAL:  return "OUTPUT_FORMAT_TERMINAL";
	case OUTPUT_FORMAT_PREMAKE_5: return "OUTPUT_FORMAT_PREMAKE_5";
	default:
		printf("ERROR: getstr_output_format(INVALID)\n");
		exit(1);
	}
}

void program_args_print(program_args_t* args)
{
    printf("program_args:\n");
    printf("--> compiler=%s\n", args->compiler);
    printf("--> compiler_flags=%s\n", args->compiler_flags);
    printf("--> file=%s\n", args->file);
	printf("--> output_format=%s\n", getstr_output_format(args->output_format));
	printf("--> debug_print=%s\n", (args->debug_print) ? "<true>" : "<false>");
}
