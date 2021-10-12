#ifndef PROGRAM_ARGS_H
#define PROGRAM_ARGS_H

typedef enum
{
	OUTPUT_FORMAT_TERMINAL,  // default
	OUTPUT_FORMAT_PREMAKE_5,
} output_format_t;

typedef struct
{
	char* compiler;
	char* compiler_flags;
	char* file;
	output_format_t output_format;
	int debug_print;
} program_args_t;

int program_args_parse(int argc, char* argv[], program_args_t* args);

void program_args_init(program_args_t* args);
void program_args_free(program_args_t* args);

void program_args_print(program_args_t* args);


#endif // PROGRAM_ARGS_H
