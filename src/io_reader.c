#include <stdio.h>
#include <stdlib.h> // TODO: Consider removing after removing exit() from code!
#include <string.h>
#include "module_unit.h"
#include "string_helper.h"
#include "io_reader.h"

#define TEST_PRINT_ALL 0


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

// TODO: We might care about the difference at some point,
// so we take them into account atm. and leave a note about it here.
typedef enum
{
	HEADER_TYPE_NOT_A_HEADER,
	HEADER_TYPE_LOCAL_HEADER,   // import "header"
	HEADER_TYPE_GLOBAL_HEADER   // import <header>
} header_type_t;

typedef enum
{
	MODULE_LINE_INVALID_SYNTAX,
	MODULE_LINE_DUPLICATE_DEFINITION,

	MODULE_LINE_GLOBAL_MODULE_FRAGMENT,
	MODULE_LINE_DECLARE_MODULE,
	MODULE_LINE_DECLARE_PARTITION,

	MODULE_LINE_EXPORT,

	MODULE_LINE_IMPORT_MODULE,
	MODULE_LINE_IMPORT_PARTITION,
	MODULE_LINE_IMPORT_HEADER
} module_line_t;

typedef enum
{
	MACRO_LINE_UNFINISHED,
	MACRO_LINE_FINISHED
} macro_line_t;

typedef struct
{
	int is_done;
	int module_read_error;
	int duplicate_definition_error;
	int multiline_comment;
	int multiline_macro;
	int may_not_declare_module;
	int has_global_module_fragment;
	int has_module_declaration;
	int has_export_declaration;
	int has_export_import_declaration;
} file_status_t;


// ================================= //
// === INTERNAL HELPER FUNCTIONS === //
// ================================= //

const char* get_keyword_string(keyword_t keyword)
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

const char* get_module_line_string(module_line_t line)
{
	switch (line)
	{
	case MODULE_LINE_INVALID_SYNTAX:         return "MODULE_LINE_INVALID_SYNTAX";
	case MODULE_LINE_DUPLICATE_DEFINITION:   return "MODULE_LINE_DUPLICATE_DEFINITION";
	case MODULE_LINE_GLOBAL_MODULE_FRAGMENT: return "MODULE_LINE_GLOBAL_MODULE_FRAGMENT";
	case MODULE_LINE_DECLARE_MODULE:    return "MODULE_LINE_DECLARE_MODULE";
	case MODULE_LINE_DECLARE_PARTITION: return "MODULE_LINE_DECLARE_PARTITION";
	case MODULE_LINE_EXPORT:            return "MODULE_LINE_EXPORT";
	case MODULE_LINE_IMPORT_MODULE:     return "MODULE_LINE_IMPORT_MODULE";
	case MODULE_LINE_IMPORT_PARTITION:  return "MODULE_LINE_IMPORT_PARTITION";
	case MODULE_LINE_IMPORT_HEADER:     return "MODULE_LINE_IMPORT_HEADER";
	default:
		return "ERROR: get_module_line_string(INVALID)";
	}
}

static void init_file_status(file_status_t* status)
{
	status->is_done = 0;
	status->module_read_error = 0;
	status->duplicate_definition_error = 0;
	status->multiline_comment = 0;
	status->multiline_macro = 0;
	status->may_not_declare_module = 0;
	status->has_global_module_fragment = 0;
	status->has_module_declaration = 0;
	status->has_export_declaration = 0;
	status->has_export_import_declaration = 0;
}

void print_file_status(file_status_t* status)
{
	printf("file_status:\n");
	printf("--> is_done = %i\n", status->is_done);
	printf("--> module_read_error = %i\n", status->module_read_error);
	printf("--> multiline_comment = %i\n", status->multiline_comment);
	printf("--> has_global_module_fragment = %i\n", status->has_global_module_fragment);
	printf("--> has_module_declaration = %i\n", status->has_module_declaration);
	printf("--> is_done = %i\n", status->has_export_declaration);
	printf("--> has_export_import_declaration = %i\n", status->has_export_import_declaration);
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
	while (strhlp_is_whitespace(*ptr)) {
		ptr++;
	}
	*read_ptr = ptr;

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

	if (strncmp(ptr, "export", 6) == 0) {
		ptr = &ptr[6];
		if (strhlp_ends_keyword_export(*ptr)) {
			*read_ptr = ptr;
			return KEYWORD_EXPORT;
		}
	}
	else if (strncmp(ptr, "import", 6) == 0) {
		ptr = &ptr[6];
		if (strhlp_ends_keyword_import(*ptr)) {
			*read_ptr = ptr;
			return KEYWORD_IMPORT;
		}
	}
	else if (strncmp(ptr, "module", 6) == 0) {
		ptr = &ptr[6];
		if (strhlp_ends_keyword_module(*ptr)) {
			*read_ptr = ptr;
			return KEYWORD_MODULE;
		}
	}

	// We did not read a module keyword or a token, so we try to
	// read a symbol instead.
	*read_ptr = ptr;
	return KEYWORD_SYMBOL;
}


static macro_line_t read_macro_line(char** read_ptr)
{
	// If one found, macro continues to next line, but only when it's the
	// last character on that line (before newline that is).
	int has_backslash = 0;

	char* ptr = *read_ptr;

	while (*ptr != '\0')
	{
		if (strhlp_is_newline(*ptr) || *ptr == ' ') {}
		else if (*ptr == '\\') { has_backslash = 1; }
		else { has_backslash = 0; }
		ptr++;
	}

	if (has_backslash) return MACRO_LINE_UNFINISHED;
	else return MACRO_LINE_FINISHED;
}


static module_line_t read_module_line(keyword_t start_keyword, char** read_ptr,
									  module_unit_t* unit, file_status_t* status)
{
	int endline = 0;
	int line_should_end = 0;

	const int max_line_keywords = 6;
	keyword_t keywords[max_line_keywords];
	keywords[0] = start_keyword;
	for (int i = 1; i < max_line_keywords; i++) keywords[i] = KEYWORD_NULL;

	const int max_line_symbols = 2;
	char* symbols[max_line_symbols];
	for (int i = 0; i < max_line_symbols; i++) symbols[i] = NULL;

	header_type_t headers[max_line_symbols];
	for (int i = 0; i < max_line_symbols; i++) headers[i] = HEADER_TYPE_NOT_A_HEADER;

	int keyword_pos = 1;
	int symbol_pos = 0;


	while (!endline)
	{
		keyword_t keyword = read_keyword(read_ptr);

		if (line_should_end || keyword_pos >= max_line_keywords) {
			switch (keyword)
			{
			case KEYWORD_NULL:
			case KEYWORD_ENDLINE:
			case KEYWORD_SEMICOLON:
			case KEYWORD_SINGLELINE_COMMENT:
			case KEYWORD_MULTILINE_COMMENT_BEGIN:
				endline = 1;
				continue;
			default:
				return MODULE_LINE_INVALID_SYNTAX;
			}
		}

		switch (keyword)
		{
		case KEYWORD_NULL:
		case KEYWORD_ENDLINE:
			endline = 1;
			break;

		case KEYWORD_HASHTAG:
			printf("read_module_line: KEYWORD_HASHTAG\n");
			return MODULE_LINE_INVALID_SYNTAX;

		case KEYWORD_COLON:
			keywords[keyword_pos++] = KEYWORD_COLON;
			break;
		case KEYWORD_SEMICOLON:
			keywords[keyword_pos++] = KEYWORD_SEMICOLON;
			line_should_end = 1;
			continue;

		case KEYWORD_SINGLELINE_COMMENT:
			printf("read_module_line: KEYWORD_SINGLELINE_COMMENT not implemented!\n");
			return MODULE_LINE_INVALID_SYNTAX;
		case KEYWORD_MULTILINE_COMMENT_BEGIN:
			// TODO: Search for commend-end. And if no such exists in line error
			printf("read_module_line: KEYWORD_MULTILINE_COMMENT_BEGIN not implemented!\n");
			return MODULE_LINE_INVALID_SYNTAX;
		case KEYWORD_MULTILINE_COMMENT_END:
			printf("read_module_line: KEYWORD_MULTILINE_COMMENT_END not implemented!\n");
			return MODULE_LINE_INVALID_SYNTAX;

		case KEYWORD_EXPORT:
			// `export` may only appear as the first word in a line, so this is
			// invalid syntax.
			return MODULE_LINE_INVALID_SYNTAX;
		case KEYWORD_IMPORT:
			keywords[keyword_pos++] = KEYWORD_IMPORT;
			break;
		case KEYWORD_MODULE:
			keywords[keyword_pos++] = KEYWORD_MODULE;
			break;

		case KEYWORD_SYMBOL:
			if (keywords[0] == KEYWORD_EXPORT && keyword_pos == 1) {
				endline = 1;
				continue;
			}
			if (symbol_pos >= max_line_symbols) {
				return MODULE_LINE_INVALID_SYNTAX;
			}

			switch ((*read_ptr)[0]) {
			case '<':
				headers[symbol_pos] = HEADER_TYPE_GLOBAL_HEADER;
				(*read_ptr)++;
				break;
			case '\"':
				headers[symbol_pos] = HEADER_TYPE_LOCAL_HEADER;
				(*read_ptr)++;
				break;
			default:
				break;
			}

			int len;
			char* symbol = strhlp_read_module_name(*read_ptr, &len);
			if (symbol == NULL) {
				return MODULE_LINE_INVALID_SYNTAX;
			}
			if (headers[symbol_pos] != HEADER_TYPE_NOT_A_HEADER) {
				len += 1; // account for '\"' or '>' following header name
			}
			symbols[symbol_pos++] = symbol;
			keywords[keyword_pos++] = KEYWORD_SYMBOL;
			*read_ptr = &((*read_ptr)[len]);
			break;

		default:
			printf("read_module_line: Invalid keyword!\n");
			exit(1);
		}
	}

	// debug print
#if TEST_PRINT_ALL
	printf(">%i keywords:\n", keyword_pos);
	for (int i = 0; i < max_line_keywords; i++) {
		printf("--> [%i]: %s\n", i, get_keyword_string(keywords[i]));
	}
	printf(">%i symbols:\n", symbol_pos);
	for (int i = 0; i < max_line_symbols; i++) {
		printf("--> [%i]: \"%s\"\n", i, symbols[i]);
	}
#endif

	// gather line data

	// "export ..."
	if (keywords[0] == KEYWORD_EXPORT) {
		// "export module ..."
		if (keyword_pos >= 3 && keywords[1] == KEYWORD_MODULE &&
			symbol_pos >= 1 && keywords[2] == KEYWORD_SYMBOL) {
			// module unit has already been declared for this file
			if (status->has_module_declaration) {
				return MODULE_LINE_DUPLICATE_DEFINITION;
			}

			// "export module <symbol> : <symbol> ;"
			if (keyword_pos >= 6 && keywords[2] == KEYWORD_SYMBOL && symbol_pos >= 2 &&
				keywords[3] == KEYWORD_COLON && keywords[4] == KEYWORD_SYMBOL &&
				keywords[5] == KEYWORD_SEMICOLON) {
				unit->module_name = symbols[0];
				unit->partition_name = symbols[1];
				unit->module_type = MODULE_TYPE_PARTITION;
				status->has_module_declaration = 1;
				return MODULE_LINE_DECLARE_PARTITION;
			}
			// "export module <symbol> ;"
			else if (keyword_pos >= 4 && keywords[3] == KEYWORD_SEMICOLON && symbol_pos >= 1) {
				unit->module_name = symbols[0];
				unit->module_type = MODULE_TYPE_MODULE;
				status->has_module_declaration = 1;
				return MODULE_LINE_DECLARE_MODULE;
			}
			else {
				return MODULE_LINE_INVALID_SYNTAX;
			}
		}
		// "export import ..."
		else if (keyword_pos >= 3 && keywords[1] == KEYWORD_IMPORT) {
			// "export import <symbol> ;"
			if (keyword_pos >= 4 && keywords[2] == KEYWORD_SYMBOL &&
				keywords[3] == KEYWORD_SEMICOLON && symbol_pos >= 1) {
				module_unit_addimport_module(unit, symbols[0]);
				return MODULE_LINE_IMPORT_MODULE;
			}
			// "export import : <symbol> ;"
			else if (keyword_pos >= 5 && keywords[2] == KEYWORD_COLON &&
					 keywords[3] == KEYWORD_SYMBOL && keywords[4] == KEYWORD_SEMICOLON &&
					 symbol_pos >= 1) {
				module_unit_addimport_partition(unit, symbols[0]);
				return MODULE_LINE_IMPORT_PARTITION;
			}
			else {
				return MODULE_LINE_INVALID_SYNTAX;
			}
		}
		// "export ..."
		else {
			status->has_export_declaration = 1;
			status->is_done = 1;
			return MODULE_LINE_EXPORT;
		}
	}
	// "import ..."
	else if (keywords[0] == KEYWORD_IMPORT) {
		// "import : <symbol> ;"
		if (keyword_pos >= 4 && keywords[1] == KEYWORD_COLON &&
			keywords[2] == KEYWORD_SYMBOL && keywords[3] == KEYWORD_SEMICOLON
			&& symbol_pos >= 1) {
			module_unit_addimport_partition(unit, symbols[0]);
			return MODULE_LINE_IMPORT_PARTITION;
		}
		// "import <symbol> ;"
		else if (keyword_pos >= 3 && keywords[1] == KEYWORD_SYMBOL &&
				 keywords[2] == KEYWORD_SEMICOLON && symbol_pos >= 1) {
			// import is another module
			if (headers[0] == HEADER_TYPE_NOT_A_HEADER) {
				module_unit_addimport_module(unit, symbols[0]);
				return MODULE_LINE_IMPORT_MODULE;
			}
			// import is a header
			// TODO: Here we may distinguish btw. local/global header import!
			else {
				module_unit_addimport_header(unit, symbols[0]);
				return MODULE_LINE_IMPORT_HEADER;
			}
		}
		else {
			return MODULE_LINE_INVALID_SYNTAX;
		}
	}
	else if (keywords[0] == KEYWORD_MODULE) {
		// "module ;"
		if (keyword_pos >= 2 && keywords[1] == KEYWORD_SEMICOLON) {
			if (status->has_global_module_fragment) {
				return MODULE_LINE_DUPLICATE_DEFINITION;
			}
			status->has_global_module_fragment = 1;
			return MODULE_LINE_GLOBAL_MODULE_FRAGMENT;
		}
		// "module <symbol> : <symbol> ;"
		else if (keyword_pos >= 5 && keywords[1] == KEYWORD_SYMBOL &&
			keywords[2] == KEYWORD_COLON && keywords[3] == KEYWORD_SYMBOL &&
			keywords[4] == KEYWORD_SEMICOLON && symbol_pos >= 2) {
			if (status->has_module_declaration) {
				return MODULE_LINE_DUPLICATE_DEFINITION;
			}
			unit->module_name = symbols[0];
			unit->partition_name = symbols[1];
			unit->module_type = MODULE_TYPE_PARTITION;
			status->has_module_declaration = 1;
			return MODULE_LINE_DECLARE_PARTITION;
		}
		// TODO: private module fragment: "module : private ;"
		else {
			return MODULE_LINE_INVALID_SYNTAX;
		}
	}


	return MODULE_LINE_INVALID_SYNTAX;
}

static void read_line(char* line, module_unit_t* unit, file_status_t* status)
{
	// `line` buffer now contains the line.
	// We read one char at a time, in search for module keywords, macros,
	// or multi-line comments.
	char* read_ptr = line;

	if (status->multiline_comment) {
		keyword_t ml_status = end_multiline_comment(&read_ptr);
		if (ml_status == KEYWORD_ENDLINE) {
			return;
		}
		else {
			status->multiline_comment = 0;
		}
	}
	if (status->multiline_macro) {
		macro_line_t ml_status = read_macro_line(&read_ptr);
		if (ml_status == MACRO_LINE_FINISHED) {
			status->multiline_macro = 0;
		}
		return;
	}

	while (1)
	{
		keyword_t keyword = read_keyword(&read_ptr);
		module_line_t module_line;
		macro_line_t macro_line;

		switch (keyword)
		{
		case KEYWORD_NULL:
		case KEYWORD_ENDLINE:
			return;

		case KEYWORD_HASHTAG:
			// Rest of line needs to be ignored
			// TODO: Macro functions may be multi-line, so we are not done, unfortunately!
			// TODO: We might not care!
			if (!status->has_global_module_fragment && !status->has_module_declaration) {
				status->may_not_declare_module = 1;
			}
			macro_line = read_macro_line(&read_ptr);
			if (macro_line == MACRO_LINE_UNFINISHED) {
				status->multiline_macro = 1;
			}
			return;
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
			if (status->may_not_declare_module &&
				(keyword == KEYWORD_EXPORT || keyword == KEYWORD_MODULE)) {
				status->module_read_error = 1;
				return;
			}
			module_line = read_module_line(keyword, &read_ptr, unit, status);
			switch (module_line)
			{
			case MODULE_LINE_INVALID_SYNTAX:
				// We encountered a syntax error in a module line.
				// The entire file will be considered invalid.
#if TEST_PRINT_ALL
				printf("MODULE_LINE_INVALID_SYNTAX\n");
				printf("line: %s", line);
#endif
				status->module_read_error = 1;
				return;
			case MODULE_LINE_DUPLICATE_DEFINITION:
#if TEST_PRINT_ALL
				printf("MODULE_LINE_DUPLICATE_DEFINITION\n");
				printf("line: %s", line);
#endif
				status->duplicate_definition_error = 1;
				return;

			case MODULE_LINE_GLOBAL_MODULE_FRAGMENT:
				status->has_global_module_fragment = 1;
				return;
			case MODULE_LINE_DECLARE_MODULE:
				status->has_module_declaration = 1;
				return;
			case MODULE_LINE_DECLARE_PARTITION:
				status->has_module_declaration = 1;
				return;

			case MODULE_LINE_EXPORT:
				// If we have seen an export declaration, and no invalid module
				// syntax has been detected, we may skip the rest of the file.
				status->is_done = 1;
				status->has_export_declaration = 1;
				return;

			case MODULE_LINE_IMPORT_MODULE:
				return;
			case MODULE_LINE_IMPORT_PARTITION:
				return;
			case MODULE_LINE_IMPORT_HEADER:
			default:
				return;
			}
			break;

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
		line_num++;
		read_line(line, unit, &status);
		if (status.is_done || status.module_read_error) break;
	}

	// free resources
	fclose(fp);
	free(line);

	// gather file data
	unit->line_num = line_num;
	unit->filename = strdup(filename);
#if TEST_PRINT_ALL
	print_file_status(&status);
#endif
	if (status.module_read_error) {
		unit->module_type = MODULE_TYPE_INVALID;
		return READ_STATUS_INVALID_SYNTAX;
	}
	else if (status.duplicate_definition_error) {
		unit->module_type = MODULE_TYPE_INVALID;
		return READ_STATUS_DUPLICATE_DEFINITION;
	}
	else if (status.has_global_module_fragment && !status.has_module_declaration) {
		unit->module_type = MODULE_TYPE_INVALID;
		return READ_STATUS_INVALID_SYNTAX;
	}
	else if ((status.has_export_declaration || status.has_export_import_declaration)
			 && !status.has_module_declaration) {
		unit->module_type = MODULE_TYPE_INVALID;
		return READ_STATUS_INVALID_SYNTAX;
	}
	else if (status.has_module_declaration) {
		return READ_STATUS_MODULE;
	}
	else {
		return READ_STATUS_NOT_MODULE;
	}
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
