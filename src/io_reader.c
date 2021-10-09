#include <stdio.h>
#include <stdlib.h> // TODO: Consider removing after removing exit() from code!
#include <string.h>
#include "module_unit.h"
#include "string_helper.h"
#include "io_reader.h"


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
	KEYWORD_MODULE,

	// Symbol name
	KEYWORD_SYMBOL
} keyword_t;

typedef enum
{
	MODULE_LINE_INVALID_SYNTAX,

	MODULE_LINE_GLOBAL_MODULE_FRAGMENT,
	MODULE_LINE_DECLARE_MODULE,
	MODULE_LINE_DECLARE_PARTITION,

	MODULE_LINE_EXPORT,

	MODULE_LINE_IMPORT_MODULE,
	MODULE_LINE_IMPORT_PARTITION,
	MODULE_LINE_IMPORT_HEADER
} module_line_t;

typedef struct
{
	read_status_t read_status;
	int is_done;

	int module_read_error;
	int file_not_module;
	int multiline_comment;
	int has_global_module_fragment;
	int has_module_declaration;
	int has_export_declaration;
} file_status_t;


// ================================= //
// === INTERNAL HELPER FUNCTIONS === //
// ================================= //

static const char* get_keyword_string(keyword_t keyword)
{
	switch (keyword)
	{
	case KEYWORD_NULL:      return "KEYWORD_NULL";
	case KEYWORD_ENDLINE:   return "KEYWORD_ENDLINE";
	case KEYWORD_HASHTAG:   return "KEYWORD_HASHTAG";
	case KEYWORD_COLON:     return "KEYWORD_COLON";
	case KEYWORD_SEMICOLON: return "KEYWORD_SEMICOLON";
	case KEYWORD_SINGLELINE_COMMENT:      return "KEYWORD_SINGLELINE_COMMENT";
	case KEYWORD_MULTILINE_COMMENT_BEGIN: return "KEYWORD_MULTILINE_COMMENT_BEGIN";
	case KEYWORD_MULTILINE_COMMENT_END:   return "KEYWORD_MULTILINE_COMMENT_END";
	case KEYWORD_EXPORT: return "KEYWORD_EXPORT";
	case KEYWORD_IMPORT: return "KEYWORD_IMPORT";
	case KEYWORD_MODULE: return "KEYWORD_MODULE";
	case KEYWORD_SYMBOL: return "KEYWORD_SYMBOL";
	default:
		return "ERROR: get_keyword_string(INVALID)";
	}
}

static void init_file_status(file_status_t* status)
{
	// Return status, initially set to READ_STATUS_NOT_MODULE
	// to take into account empty files.
	status->read_status = READ_STATUS_NOT_MODULE;
	status->is_done = 0;

	status->module_read_error = 0; // TODO: Replace with `is_done`
	status->file_not_module = 0; // TODO: Replace with `is_done`
	status->multiline_comment = 0;
	status->has_global_module_fragment = 0;
	status->has_module_declaration = 0;
	status->has_export_declaration = 0; // TODO: Replace with `is_done`
}

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

	if (strncmp(*read_ptr, "export", 6) == 0) {
		// TODO: ptr = &ptr[6];
		for (int i = 0; i < 6; i++) ptr++;
		if (strhlp_ends_keyword_export(*ptr)) {
			*read_ptr = ptr;
			return KEYWORD_EXPORT;
		}
	}
	else if (strncmp(*read_ptr, "import", 6) == 0) {
		for (int i = 0; i < 6; i++) ptr++;
		if (strhlp_ends_keyword_import(*ptr)) {
			*read_ptr = ptr;
			return KEYWORD_IMPORT;
		}
	}
	else if (strncmp(*read_ptr, "module", 6) == 0) {
		for (int i = 0; i < 6; i++) ptr++;
		if (strhlp_ends_keyword_module(*ptr)) {
			*read_ptr = ptr;
			return KEYWORD_MODULE;
		}
	}

	// We did not read a module keyword or a token, so we try to
	// read a symbol instead.
	//ptr = *read_ptr;
	return KEYWORD_SYMBOL;
}

/*
typedef enum
{
	MODULE_LINE_INVALID_SYNTAX,

	MODULE_LINE_GLOBAL_MODULE_FRAGMENT,
	MODULE_LINE_DECLARE_MODULE,
	MODULE_LINE_DECLARE_PARTITION,

	MODULE_LINE_EXPORT,

	MODULE_LINE_IMPORT_MODULE,
	MODULE_LINE_IMPORT_PARTITION,
	MODULE_LINE_IMPORT_HEADER
} module_line_t;
*/

static module_line_t read_module_line(keyword_t keyword_1, char** read_ptr,
									  module_unit_t* unit)
{
	module_line_t status = MODULE_LINE_INVALID_SYNTAX;
	int endline = 0;
	int line_should_end;

	keyword_t keyword_2 = KEYWORD_NULL;
	//keyword_t keyword_3 = KEYWORD_NULL;
	//keyword_t keyword_4 = KEYWORD_NULL;
	//keyword_t keyword_5 = KEYWORD_NULL;

	while (!endline)
	{
		keyword_t keyword = read_keyword(read_ptr);

		if (line_should_end) {
			switch (keyword)
			{
			case KEYWORD_NULL:
			case KEYWORD_ENDLINE:
			case KEYWORD_SEMICOLON:
				return status;
			default:
				return MODULE_LINE_INVALID_SYNTAX;
			}
		}

		int len;
		char* symbol;

		switch (keyword)
		{
		case KEYWORD_NULL:
		case KEYWORD_ENDLINE:
			endline = 1;
			break;

		case KEYWORD_HASHTAG:
			endline = 1;
			status = MODULE_LINE_INVALID_SYNTAX;
			break;

		case KEYWORD_COLON:
			printf("read_module_line: KEYWORD_COLON not implemented!\n");
			break;
		case KEYWORD_SEMICOLON:
			// This could be a global module fragment
			if (keyword_1 == KEYWORD_MODULE && keyword_2 == KEYWORD_NULL) {
				status = MODULE_LINE_GLOBAL_MODULE_FRAGMENT;
			}
			line_should_end = 1;
			break;

		case KEYWORD_SINGLELINE_COMMENT:
			printf("read_module_line: KEYWORD_SINGLELINE_COMMENT not implemented!\n");
			break;
		case KEYWORD_MULTILINE_COMMENT_BEGIN:
			// TODO: Search for commend-end. And if no such exists in line error
			printf("read_module_line: KEYWORD_MULTILINE_COMMENT_BEGIN not implemented!\n");
			break;
		case KEYWORD_MULTILINE_COMMENT_END:
			printf("read_module_line: KEYWORD_MULTILINE_COMMENT_END not implemented!\n");
			break;

		case KEYWORD_EXPORT:
			printf("read_module_line: KEYWORD_EXPORT not implemented!\n");
			break;
		case KEYWORD_IMPORT:
			printf("read_module_line: KEYWORD_IMPORT not implemented!\n");
			break;
		case KEYWORD_MODULE:
			printf("read_module_line: KEYWORD_MODULE not implemented!\n");
			break;

		case KEYWORD_SYMBOL:
			symbol = strhlp_read_module_name(*read_ptr, &len);
			if (symbol == NULL) {
				return status = MODULE_LINE_INVALID_SYNTAX;
			}
			else {
				printf("symbol: %s\n", symbol);
				*read_ptr = &((*read_ptr)[len]);
			}
			// TODO: Read symbol name
			printf("read_module_line: KEYWORD_SYMBOL not implemented!\n");
			break;

		default:
			printf("read_module_line: Invalid keyword!\n");
			exit(1);
		}
	}
	return status;
}

static void read_line(char* line, module_unit_t* unit, file_status_t* status)
{
	// `line` buffer now contains the line.
	// We read one char at a time, in search for module keywords, macros,
	// or multi-line comments.
	char* read_ptr = line;
	printf("read_line: %s", line);

	if (status->multiline_comment) {
		keyword_t ml_status = end_multiline_comment(&read_ptr);
		if (ml_status == KEYWORD_ENDLINE) {
			return;
		}
	}

	while (1)
	{
		keyword_t keyword = read_keyword(&read_ptr);
		printf("--> keyword: %s\n", get_keyword_string(keyword));
		module_line_t module_line;

		switch (keyword)
		{
		case KEYWORD_NULL:
		case KEYWORD_ENDLINE:
			return;

		case KEYWORD_HASHTAG:
			// Rest of line needs to be ignored
			// TODO: Macro functions may be multi-line, so we are not done, unfortunately!
			// TODO: We might not care!
			break;
		case KEYWORD_COLON:
			printf("read_file: KEYWORD_COLON not implemented!\n");
			exit(1);
		case KEYWORD_SEMICOLON:
			printf("read_file: KEYWORD_SEMICOLON not implemented!\n");
			exit(1);

		case KEYWORD_SINGLELINE_COMMENT:
			return;
		case KEYWORD_MULTILINE_COMMENT_BEGIN:
			// A multi-line comment was begun on this line, but not finished!
			// So we must skip to next line and search for comment-end.
			status->multiline_comment = 1;
			return;
		case KEYWORD_MULTILINE_COMMENT_END:
			// A multi-line comment was ended on this line, so we must search for
			// the next keyword.
			status->multiline_comment = 0;
			break;

		case KEYWORD_EXPORT:
		case KEYWORD_IMPORT:
		case KEYWORD_MODULE:
			module_line = read_module_line(keyword, &read_ptr, unit);
			switch (module_line)
			{
			case MODULE_LINE_INVALID_SYNTAX:
				// We encountered a syntax error in a module line.
				// The entire file will be considered invalid.
				status->read_status = READ_STATUS_INVALID_SYNTAX;
				return;
			case MODULE_LINE_GLOBAL_MODULE_FRAGMENT:
				if (status->has_global_module_fragment) {
					// duplicate global module fragment
					status->read_status = READ_STATUS_INVALID_SYNTAX;
					return;
				}
				else {
					status->has_global_module_fragment = 1;
				}
				break;
			case MODULE_LINE_DECLARE_MODULE:
				if (status->has_module_declaration) {
					// duplicate module declaration
					status->read_status = READ_STATUS_INVALID_SYNTAX;
					return;
				}
				else {
					status->has_module_declaration = 1;
				}
				break;
			case MODULE_LINE_DECLARE_PARTITION:
				if (status->has_module_declaration) {
					// duplicate module declaration
					status->read_status = READ_STATUS_INVALID_SYNTAX;
					return;
				}
				else {
					status->has_module_declaration = 1;
				}
				break;

			case MODULE_LINE_EXPORT:
				// If we have seen an export declaration, and no invalid module
				// syntax has been detected, we may skip the rest of the file.
				status->is_done = 1;
				return;

			case MODULE_LINE_IMPORT_MODULE:
			case MODULE_LINE_IMPORT_PARTITION:
			case MODULE_LINE_IMPORT_HEADER:
			default:
				printf("Invalid or non-implemented module-line in read_line\n");
				exit(1);
			}

		case KEYWORD_SYMBOL:
			// we are done.
			status->is_done = 1;
			return;

		default:
			printf("Invalid or non-implemented keyword in read_line\n");
			exit(1);
		}
	} // while (1)
}


// ====================== //
// === FILE INTERFACE === //
// ====================== //

read_status_t read_file(char* filename, module_unit_t* unit)
{
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) return READ_STATUS_FILE_NOT_EXISTS;

	// Reset module_unit_t struct just in case.
	module_unit_free(unit);

	// Declare line pointer here.
	// Parsing functions call `getline`, and since `line` is initially NULL,
	// `getline` will automatically allocate memory for the line, and call
	// `realloc` if at some point it reads a line which does not fit into this
	// buffer. This way we minimize memory allocation overhead.
	char* line = NULL;
	size_t len;
	ssize_t nread;
	unsigned long line_num = 0; // used for reporting errors

	// Struct used to maintain file status across line reads
	file_status_t status;
	init_file_status(&status);

	while ((nread = getline(&line, &len, fp)) != -1)
	{
		read_line(line, unit, &status);
		if (status.is_done) break;
		line_num++;
	}

	fclose(fp);
	free(line);

	// TODO: Also return number of lines read, for reporting errors
	unit->line_num = line_num;
	return status.read_status;
}


const char* get_read_status_string(read_status_t status)
{
	switch (status)
	{
	case READ_STATUS_FILE_NOT_EXISTS: return "READ_STATUS_FILE_NOT_EXISTS";
	case READ_STATUS_INVALID_SYNTAX:  return "READ_STATUS_INVALID_SYNTAX";
	case READ_STATUS_MODULE:          return "READ_STATUS_MODULE";
	case READ_STATUS_NOT_MODULE:      return "READ_STATUS_NOT_MODULE";
	default:
		return "ERROR: get_read_status_string(INVALID)";
	}
}
