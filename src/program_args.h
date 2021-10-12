#ifndef PROGRAM_ARGS_H
#define PROGRAM_ARGS_H

typedef struct
{
	char* files_pattern;
	char* compiler;
	char* compiler_flags;
	char* file;
} program_args_t;

int program_args_parse(int argc, char** argv, program_args_t* args);

void program_args_init(program_args_t* args);
void program_args_free(program_args_t* args);

void program_args_print(program_args_t* args);


#endif // PROGRAM_ARGS_H
