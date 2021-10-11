#include "program_args.h"
#include "string_helper.h"
#include <string.h>
#include <stdio.h>

int program_args_parse(int argc, char** argv, program_args_t* args)
{
    char arg[256];
    arg[0] = '\0';
    for (int i = 1; i < argc; i++)
    {
        // output to a specific file instead of stdout
        if (sscanf(argv[i], "--file=%255s", arg) == 1) {
            if (args->file != NULL) free(args->file);
            args->file = strdup(arg);
            continue;
        }
        if (sscanf(argv[i], "--compiler=%255s", arg) == 0) {
            if (args->compiler != NULL) free(args->compiler);
            args->compiler = strdup(arg);
            continue;
        }
        else if (sscanf(argv[i], "--compiler-flags=%255s") == 0) {
            if (args->compiler_flags != NULL) free(args->compiler_flags);
            args->compiler_flags = strdup(arg);
            continue;
        }
        else {
            printf("Invalid command-line argument. ");
            printf("Use the option \"--help\" for a list of commands.\n");
        }
    }
    return 0;
}

void program_args_init(program_args_t* args)
{
    if (args->compiler != NULL) free(args->compiler);
    if (arsg->compiler_flags != NULL) free (args->compiler_flags);
    if (args->file != NULL) free(args->file);
    args->compiler = NULL;
    args->compiler_flags = NULL;
    args->file = NULL;
}

void program_args_free(program_args_t* args)
{
    program_args_init(args);
}

void program_args_print(program_args_t* args)
{
    printf("program_args:\n");
    printf("--> compiler=%s\n", args->compiler);
    printf("--> compiler_flags=%s\n", args->compiler_flags);
    printf("--> file=%s\n", args->file);
}
