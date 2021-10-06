#include "io_helper.h"
#include <stdio.h>
#include <stdlib.h>

static int is_trailing_char(char c)
{
	switch(c)
	{
	case '\n':
	case '\r':
	case ';':
		return 1;
	default:
		return 0;
	}
}

static char* dup_string(const char* str)
{
	size_t len = strlen(str);
	char* mem = malloc(len + 1);
	strcpy(mem, str);
	// Remove trailing characters if any
	if (is_trailing_char(mem[len-1])) mem[len-1] = '\0';
	if (is_trailing_char(mem[len-2])) mem[len-2] = '\0';
	return mem;
}

static char* dup_string_n(const char* str, size_t n)
{
	// Hmm will this work?
	char* mem = malloc(n + 1);
	strncpy(mem, str, n);
	mem[n] = '\0';
	return mem;
}

static int has_char(const char* str, char c)
{
	char* ptr = (char*)str;
	while (*ptr != '\0')
	{
		if (*ptr == c) return 1;
		ptr++;
	}
	return 0;
}

// Dealing with multiline comments in files.
typedef enum
{
	MULTILINE_COMMENT_FINISHED,
	MULTILINE_COMMEND_UNFINISHED
} multiline_comment_t;

// TODO: This function is under-untilized
static multiline_comment_t trim_away_multiline_comment(char* start, char** trimmed)
{
	char* ptr = start;
	while (1)
	{
		if (*ptr == '\n' || *ptr == '\r' || *ptr == '\0') {
			return MULTILINE_COMMENT_UNFINISHED;
		}
		if (*ptr == '*') {
			ptr++;
			if (*ptr == '/') {
				ptr++;
				*trimmed = ptr;
				return MULTILINE_COMMENT_FINISHED;
			}
		}
		ptr++;
	}
}


void init_lineinfo(module_lineinfo_t* info)
{
	info->linetype = LINETYPE_UNDETERMINED;
	info->module_name = NULL;
	info->partition_name = NULL;
	info->header_name = NULL;
}

void free_lineinfo(module_lineinfo_t* info)
{
	printf("THIS FUNCTION SHOULD NOT BE CALLED (free_lineinfo)\n");
	info->linetype = LINETYPE_INVALID_MODULE_SYNTAX;
	if (info->module_name != NULL) free(info->module_name);
	if (info->header_name != NULL) free(info->header_name);
	if (info->partition_name != NULL) free(info->partition_name);
	info->module_name = NULL;
	info->header_name = NULL;
	info->partition_name = NULL;
}


void read_module_declaration(module_lineinfo_t* info)
{
	char* tok = strtok(NULL, "; ");
	if (tok == NULL) { return; }

	// Check if module_name has a colon, and if so split it,
	// ie. "module-name:partition_name".
	if (has_char(tok, ':'))
	{
		// First read module-name
		size_t len = 0;
		while (1)
		{
			if (tok[len] == '\0') { return; }
			if (tok[len] == ':') {
				info->module_name = dup_string_n(tok, len);
				len++;
				break;
			}
			len++;
		}

		// Then read partition-name
		char* ptr = &tok[len];
		info->partition_name = dup_string(ptr);
		info->linetype = LINETYPE_MODULE_PARTITION_DECLARATION;

		// NOTE: For now we ignore attr(optional)
		return;
	}


	info->module_name = dup_string(tok);
	tok = strtok(NULL, " ");
	// nothing more to parse
	if (tok == NULL) {
		info->linetype = LINETYPE_MODULE_DECLARATION;
		return;
	}
	else if (strcmp(tok, ";") == 0) {
		info->linetype = LINETYPE_MODULE_DECLARATION;
		return;
	}
	// check partition name
	else if (strcmp(tok, ":") == 0) {
		tok = strtok(NULL, " ");
		if (tok == NULL) { return; }

		info->partition_name = dup_string(tok);
		// NOTE: This would be the place to check attr(optional)
		// TODO: Might also report invalid syntax if semicolon is missing
		info->linetype = LINETYPE_MODULE_PARTITION_DECLARATION;
		return;
	}
	else if (tok[0] == ':') {
		// We need to split ':' from the partition name
		char* ptr = &tok[1];
		if (*ptr == '\0') { return; }

		info->partition_name = dup_string(ptr);
		info->linetype = LINETYPE_MODULE_PARTITION_DECLARATION;
		return;
	}
	// NOTE: For now we ignore attr(optional)
	else {
		// Nothing more to be done here.
		info->linetype = LINETYPE_MODULE_DECLARATION;
		return;
	}
}


/*
 * This function is called after reading "import".
 */
static void read_import_declaration(module_lineinfo_t* info)
{
	char* tok = strtok(NULL, "; ");
	if (tok == NULL) { return; }

	// we might import a partition
	if (tok[0] == ':') {
		char* ptr = &tok[1];
		if (*ptr == '\0') { return; }

		info->partition_name = dup_string(ptr);
		info->linetype = LINETYPE_IMPORT_PARTITION;
		return;
	}
	// there is space between ':' and partition name
	else if (strcmp(tok, ":") == 0) {
		tok = strtok(NULL, "; ");
		if (tok == NULL) { return; }

		info->partition_name = dup_string(tok);
		info->linetype = LINETYPE_IMPORT_PARTITION;
		return;
	}
	// TODO: Check if we import a header

	// we import a module
	else {
		info->module_name = dup_string(tok);
		info->linetype = LINETYPE_IMPORT_MODULE;

		// NOTE: For now we ignore attr(optional)
		return;
	}
}


void read_line(char* line, module_lineinfo_t* info)
{
	info->linetype = LINETYPE_INVALID_MODULE_SYNTAX;
	char* tok = strtok(line, "\t\n ");

	// If the first token is "module" it is either a global
	// or private module fragment, or a module declaration.
	if(tok == NULL) {
		info->linetype = LINETYPE_EMPTY;
		return;
	}
	else if (strcmp(tok, "module") == 0)
	{
		tok = strtok(NULL, " ");

		if (tok == NULL) { return; }
		else if (strcmp(tok, ";") == 0) {
			info->linetype = LINETYPE_GLOBAL_MODULE_FRAGMENT;
			return;
		}
		else if (strcmp(tok, ":") == 0) {
			info->linetype = LINETYPE_PRIVATE_MODULE_FRAGMENT;
			return;
		}

		// module declaration, first check module name
		read_module_declaration(info);
		return;
	}
	else if (strcmp(tok, "module;") == 0) {
		info->linetype = LINETYPE_GLOBAL_MODULE_FRAGMENT;
		return;
	}

	// If first word is "export" it can be an import declaration,
	// a module declaration, or an export declaration.
	else if(strcmp(tok, "export") == 0) {
		tok = strtok(NULL, " ");
		if (tok == NULL) { return; }

		// import declaration
		if (strcmp(tok, "import") == 0) {
			read_import_declaration(info);
			return;
		}
		// module declaration
		else if (strcmp(tok, "module") == 0) {
			read_module_declaration(info);
			return;
		}
		// export declaration.
		// At this point, no further parsing needs to be done!
		else {
			info->linetype = LINETYPE_EXPORT_DECLARATION;
			return;
		}
	}
	// import declaration, without exporting (but we don't care :) )
	else if (strcmp(tok, "import") == 0) {
		read_import_declaration(info);
		return;
	} // TODO: "Only preprocessing directives can appear in the global module fragment."
	else { // TODO: Possibly add check for lines starting with '#', might carry significance?
		info->linetype = LINETYPE_OTHER;
		return;
	}
}


static io_read_status_t read_module_unit_info(FILE* fp, module_unit_t* module_unit, char** line)
{
	io_read_status_t status = IO_READ_STATUS_INVALID_MODULE_SYNTAX;

	int has_error = 0;
	int should_stop = 0;
	int has_global_module_fragment_declared = 0;
	int has_module_unit_declared = 0;

	size_t len;
	ssize_t nread;
	// NOTE: We can assume that a module line will never be longer than 1024.
	// This assumption is entirely valid for the module syntax.
	// Most, if not all, lines will be < 100 characters.
	while ((nread = getline(line, &len, fp)) != -1)
	{
		//printf("line: %s", (*line == NULL) ? "<null>" : *line);
		if (has_error || should_stop) break;

		// Create struct to hold info about current line in file.
		module_lineinfo_t info;
		init_lineinfo(&info);

		// Parse current line in file.
		read_line(*line, &info);

		// Perform an action based on current line info, based on currently maintained
		// read state.
		switch(info.linetype)
		{
		case LINETYPE_OTHER:
			// TODO: Is this right?
			// TODO: Possibly not - should take into account comment lines.
			if (has_module_unit_declared) {
				should_stop = 1;
			}
			break;
		case LINETYPE_EMPTY:
			break;
		case LINETYPE_INVALID_MODULE_SYNTAX:
			status = IO_READ_STATUS_INVALID_MODULE_SYNTAX;
			has_error = 1;
			break;

		case LINETYPE_MODULE_DECLARATION:
			if (has_module_unit_declared) {
				status = IO_READ_STATUS_DUPLICATE_DEFINITION;
				has_error = 1;
			} else {
				module_unit->module_type = MODULE_TYPE_MODULE;
				module_unit->module_name = info.module_name;
				status = IO_READ_STATUS_MODULE;
				has_module_unit_declared = 1;
			}
			break;

		case LINETYPE_MODULE_PARTITION_DECLARATION:
			if (has_module_unit_declared) {
				status = IO_READ_STATUS_DUPLICATE_DEFINITION;
				has_error = 1;
			} else {
				module_unit->module_type = MODULE_TYPE_PARTITION;
				module_unit->module_name = info.module_name;
				module_unit->partition_name = info.partition_name;
				status = IO_READ_STATUS_MODULE_PARTITION;
				has_module_unit_declared = 1;
			}
			break;

		case LINETYPE_EXPORT_DECLARATION:
			should_stop = 1;
			break;

		case LINETYPE_IMPORT_MODULE:
			module_unit_addimport_module(module_unit, info.module_name);
			break;
		case LINETYPE_IMPORT_PARTITION:
			module_unit_addimport_partition(module_unit, info.partition_name);
			break;
		case LINETYPE_IMPORT_HEADER:
			module_unit_addimport_header(module_unit, info.header_name);
			break;

		case LINETYPE_GLOBAL_MODULE_FRAGMENT:
			if (has_global_module_fragment_declared) {
				status = IO_READ_STATUS_DUPLICATE_DEFINITION;
				has_error = 1;
			} else {
				has_global_module_fragment_declared = 1;
			}
			break;

		case LINETYPE_PRIVATE_MODULE_FRAGMENT:
			status = IO_READ_STATUS_UNSUPPORTED_DECLARATION;
			has_error = 1;
			break;
		default:
			printf("ERROR: Invalid linetype\n");
			status = IO_READ_STATUS_INTERNAL_ERROR;
			has_error = 1;
			break;
		}
	} // while(...)

	return status;
}


/*
 * Perform a quick check to see, if the file is actually a module unit.
 */
static int is_file_module_unit(FILE* fp, char** line)
{
	size_t len;
	ssize_t nread;
	// NOTE: We can assume that a module line will never be longer than 1024.
	// This assumption is entirely valid for the module syntax.
	// Most, if not all, lines will be < 100 characters.
	while ((nread = getline(line, &len, fp)) != -1)
	{
		char* tok = strtok(*line, "; \n");
		if ((strcmp(tok, "module") == 0) || (strcmp(tok, "export") == 0)) return 1;
		else if (len <= 2 || tok[0] == '/') continue;
		else return 0;
	}
	return 0;
}


io_read_status_t parse_file(char* filename, module_unit_t* unit)
{
	io_read_status_t status = IO_READ_STATUS_UNDETERMINED;

	FILE* fp = fopen(filename, "r");
	if (!fp) return IO_READ_STATUS_FILE_NOT_EXISTS; // TODO: errno and strerr

	// Declare line pointer here.
	// Parsing functions call `getline`, and since `line` is initially NULL,
	// `getline` will automatically allocate memory for the line, and call
	// `realloc` if at some point it reads a line which does not fit into this
	// buffer.
	// This way we minimize memory allocation overhead.
	char* line = NULL;
	if (is_file_module_unit(fp, &line))
	{
		// rewind file pointer!
		rewind(fp);

		// parse and fill out `unit` struct.
		status = read_module_unit_info(fp, unit, &line);
		unit->filename = dup_string(filename);
	}
	else
	{
		status = IO_READ_STATUS_NOT_MODULE;
	}

	fclose(fp);
	free(line);
	return status;
}




const char* get_linetype_string(module_linetype_t lt)
{
	switch(lt)
	{
	case LINETYPE_UNDETERMINED:
		return "LINETYPE_UNDETERMINED";
	case LINETYPE_OTHER:
		return "LINETYPE_OTHER";
	case LINETYPE_EMPTY:
		return "LINETYPE_EMPTY";
	case LINETYPE_PREPROCESSING_DIRECTIVE:
		return "LINETYPE_PREPROCESSING_DIRECTIVE";

	case LINETYPE_INVALID_MODULE_SYNTAX:
		return "LINETYPE_INVALID_MODULE_SYNTAX";

	case LINETYPE_MODULE_DECLARATION:
		return "LINETYPE_MODULE_DECLARATION";
	case LINETYPE_MODULE_PARTITION_DECLARATION:
		return "LINETYPE_MODULE_PARTITION_DECLARATION";

	case LINETYPE_EXPORT_DECLARATION:
		return "LINETYPE_EXPORT_DECLARATION";

	case LINETYPE_IMPORT_MODULE:
		return "LINETYPE_IMPORT_MODULE";
	case LINETYPE_IMPORT_PARTITION:
		return "LINETYPE_IMPORT_PARTITION";
	case LINETYPE_IMPORT_HEADER:
		return "LINETYPE_IMPORT_HEADER";

	case LINETYPE_GLOBAL_MODULE_FRAGMENT:
		return "LINETYPE_GLOBAL_MODULE_FRAGMENT";
	case LINETYPE_PRIVATE_MODULE_FRAGMENT:
		return "LINETYPE_PRIVATE_MODULE_FRAGMENT";

	default:
		return "ERROR: Unrecognized linetype";
	}
}

const char* get_io_status_string(io_read_status_t rs)
{
	switch (rs)
	{
	case IO_READ_STATUS_UNDETERMINED:
		return "IO_READ_STATUS_UNDETERMINED";
	case IO_READ_STATUS_FILE_NOT_EXISTS:
		return "IO_READ_STATUS_FILE_NOT_EXISTS";
	case IO_READ_STATUS_INTERNAL_ERROR:
		return "IO_READ_STATUS_INTERNAL_ERROR";
	case IO_READ_STATUS_NOT_MODULE:
		return "IO_READ_STATUS_NOT_MODULE";
	case IO_READ_STATUS_MODULE:
		return "IO_READ_STATUS_MODULE";
	case IO_READ_STATUS_MODULE_PARTITION:
		return "IO_READ_STATUS_MODULE_PARTITION";
	case IO_READ_STATUS_INVALID_MODULE_SYNTAX:
		return "IO_READ_STATUS_INVALID_MODULE_SYNTAX";
	case IO_READ_STATUS_UNSUPPORTED_DECLARATION:
		return "IO_READ_STATUS_UNSUPPORTED_DECLARATION";
	default:
		return "ERROR: Unrecognized io_status";
	}
}
