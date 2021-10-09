#include <stdio.h>
#include "module_unit.h"
#include "string_helper.h"
#include "new_io_reader.h"


typedef enum
{
	KEYWORD_NULL,
	KEYWORD_ENDLINE,

	KEYWORD_HASHTAG,
	KEYWORD_COLON,
	KEYWORD_SEMICOLON,
	KEYWORD_SINGLELINE_COMMENT,
	KEYWORD_MULTILINE_COMMENT_BEGIN,
	KEYWORD_MULTILINE_COMMENT_END,

	KEYWORD_EXPORT,
	KEYWORD_IMPORT,
	KEYWORD_MODULE
} keyword_t;

keyword_t end_multiline_comment(char** read_ptr)
{
	char* ptr = *read_ptr;
	while (1) {
		switch (*ptr) {
		case '\0':
		case '\n':
		case '\r': return KEYWORD_ENDLINE;
		default: break;
		}

		if (ptr[0] == '*' && ptr[1] == '/') {
			ptr++; ptr++;
			*read_ptr = ptr;
			return KEYWORD_MULTILINE_COMMENT_END;
		}
		ptr++;
	}
	*read_ptr = ptr;
}

keyword_t read_keyword(char** read_ptr)
{
	char* ptr = *read_ptr;
	while (string_helper_is_whitespace(*ptr)) {
		ptr++;
	}

	switch (*ptr)
	{
	case '\0': return KEYWORD_NULL;
	case '\n': return KEYWORD_ENDLINE;
	case '\r': return KEYWORD_ENDLINE;
	case '#': (*read_ptr)++; return KEYWORD_HASHTAG;
	case ':': (*read_ptr)++; return KEYWORD_COLON;
	case ';': (*read_ptr)++; return KEYWORD_SEMICOLON;
	default: break;
	}

	if (ptr[0] == '/' && ptr[1] == '*') {
		ptr++; ptr++;
		keyword_t ml_status = end_multiline_comment(&ptr);
		if (ml_status == KEYWORD_ENDLINE) {
			return KEYWORD_MULTILINE_COMMENT_BEGIN;
		}
		// otherwise, we can just continue reading from here.
	}
}


read_status_t read_file(char* filename, module_unit_t* unit)
{
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) return READ_STATUS_FILE_NOT_EXISTS;

	module_unit_free(unit);

	// Declare line pointer here.
	// Parsing functions call `getline`, and since `line` is initially NULL,
	// `getline` will automatically allocate memory for the line, and call
	// `realloc` if at some point it reads a line which does not fit into this
	// buffer.
	// This way we minimize memory allocation overhead.
	char* line = NULL;
	size_t len;
	ssize_t nread;
	while ((nread = getline(line, &len, fp)) != -1)
	{
		// `line` buffer now contains the line.
		// We read one char at a time, in search for module keywords, macros,
		// or multi-line comments.
		char* read_ptr = line;

		keyword_t keyword = read_keyword(*read_ptr);
		switch (keyword)
		{
		case KEYWORD_HASHTAG:
			// Rest of line needs to be ignored
			break;
		}
	}

	fclose(fp);
	free(line);
}
