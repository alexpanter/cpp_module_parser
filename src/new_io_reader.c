#include <stdio.h>
#include "module_unit.h"
#include "string_helper.h"
#include "new_io_reader.h"


// ============================= //
// === INTERNALLY USED TYPES === //
// ============================= //

typedef enum
{
	// Line termination
	KEYWORD_NULL,
	KEYWORD_ENDLINE,

	// Special characters
	KEYWORD_HASHTAG,
	KEYWORD_COLON,
	KEYWORD_SEMICOLON,
	KEYWORD_SINGLELINE_COMMENT,
	KEYWORD_MULTILINE_COMMENT_BEGIN,
	KEYWORD_MULTILINE_COMMENT_END,

	// Module unit keywords
	KEYWORD_EXPORT,
	KEYWORD_IMPORT,
	KEYWORD_MODULE
} keyword_t;

typedef enum
{
	MODULE_LINE_INVALID,

	MODULE_LINE_GLOBAL_MODULE_FRAGMENT,
	MODULE_LINE_DECLARE_MODULE,
	MODULE_LINE_DECLARE_PARTITION,

	MODULE_LINE_EXPORT,

	MODULE_LINE_IMPORT_MODULE,
	MODULE_LINE_IMPORT_PARTITION,
	MODULE_LINE_IMPORT_HEADER
} module_line_t;


// ================================= //
// === INTERNAL HELPER FUNCTIONS === //
// ================================= //

static keyword_t end_multiline_comment(char** read_ptr)
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

static keyword_t read_keyword(char** read_ptr)
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
		*read_ptr = ptr;
		keyword_t ml_status = end_multiline_comment(read_ptr);
		if (ml_status == KEYWORD_ENDLINE) {
			return KEYWORD_MULTILINE_COMMENT_BEGIN;
		}
		else {
			return KEYWORD_MULTILINE_COMMENT_END;
		}
	}

	if (ptr[0] == '/' && ptr[1] == '/') return KEYWORD_SINGLELINE_COMMENT;
}

static module_line_t read_module_line(keyword_t first_keyword, char** read_ptr)
{
	module_line_t status = MODULE_LINE_INVALID;
	int endline = 0;

	while (!endline)
	{
		keyword_t keyword = read_keyword(read_ptr);
	}
	return status;
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

	// Local variables needed to determine file type
	int multiline_comment = 0;
	int has_global_module_fragment = 0;
	int has_module_declaration = 0;
	int has_export_declaration = 0;

	while ((nread = getline(line, &len, fp)) != -1)
	{
		if (has_export_declaration) {
			// If we have seen an export declaration, and no invalid module
			// syntax has been detected, we may skip the rest of the file.
			break;
		}

		// TODO: This inner loop may be exported into a separate function!

		// `line` buffer now contains the line.
		// We read one char at a time, in search for module keywords, macros,
		// or multi-line comments.
		char* read_ptr = line;
		int endline = 0;

		while (!endline)
		{
			keyword_t keyword = read_keyword(*read_ptr);
			switch (keyword)
			{
			case KEYWORD_NULL:
			case KEYWORD_ENDLINE:
				break;

			case KEYWORD_HASHTAG:
				// Rest of line needs to be ignored
				// TODO: Macro functions may be multi-line, so we are not done, unfortunately!
				break;
			case KEYWORD_COLON:
				printf("read_file: KEYWORD_COLON not implemented!\n");
				exit(1);
			case KEYWORD_SEMICOLON:
				printf("read_file: KEYWORD_SEMICOLON not implemented!\n");
				exit(1);

			case KEYWORD_SINGLELINE_COMMENT:
				break;
			case KEYWORD_MULTILINE_COMMENT_BEGIN:
				// A multi-line comment was begun on this line, but not finished!
				// So we must skip to next line (recursively) and search for comment-end.
				multiline_comment = 1;
				endline = 1;
				break;
			case KEYWORD_MULTILINE_COMMENT_END:
				// A multi-line comment was ended on this line, so we must search for
				// the next keyword.
				multiline_comment = 0;
				break;

			case KEYWORD_EXPORT:
			case KEYWORD_IMPORT:
			case KEYWORD_MODULE:
				read_module_Line(keyword, read_ptr);

			default:
				printf("Invalid or non-implemented keyword in read_file\n");
				exit(1);
			}
		} // while (!endline)
	} // while(nread = getline(...) != -1)

	fclose(fp);
	free(line);
}
