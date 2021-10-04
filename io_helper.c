#include "io_helper.h"
#include <stdio.h>
#include <stdlib.h>

static char* dup_string(const char* str)
{
	size_t len = strlen(str);
	char* mem = malloc(len + 1);
	strcpy(mem, str);
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


void init_lineinfo(module_lineinfo_t* info)
{
	info->linetype = LINETYPE_INVALID_MODULE_SYNTAX;
	info->module_name = NULL;
	info->module_partition_name = NULL;
	info->header_name = NULL;
}

void free_lineinfo(module_lineinfo_t* info)
{
	info->linetype = LINETYPE_INVALID_MODULE_SYNTAX;
	if (info->module_name != NULL) free(info->module_name);
	if (info->header_name != NULL) free(info->header_name);
	if (info->module_partition_name != NULL) free(info->module_partition_name);
	info->module_name = NULL;
	info->header_name = NULL;
	info->module_partition_name = NULL;
}

/*
  static size_t count_occurences(const char* str, char c)
  {
  size_t cnt = 0;
  char* ptr = (char*)str;
  while (*ptr != '\0')
  {
  if (*ptr == c) { cnt++; }
  }
  return cnt;
  }
*/

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
		info->module_partition_name = dup_string(ptr);
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

		info->module_partition_name = dup_string(tok);
		// NOTE: This would be the place to check attr(optional)
		// TODO: Might also report invalid syntax if semicolon is missing
		info->linetype = LINETYPE_MODULE_PARTITION_DECLARATION;
		return;
	}
	else if (tok[0] == ':') {
		// We need to split ':' from the partition name
		char* ptr = &tok[1];
		if (*ptr == '\0') { return; }

		info->module_partition_name = dup_string(ptr);
		info->linetype = LINETYPE_MODULE_PARTITION_DECLARATION;
		return;
	}
	// NOTE: For now we ignore attr(optional)
	else { return; }
}


/*
 * This function is called after reading "import".
 */
void read_import_declaration(module_lineinfo_t* info)
{
	char* tok = strtok(NULL, "; ");
	if (tok == NULL) { return; }

	// we might import a partition
	if (tok[0] == ':') {
		char* ptr = &tok[1];
		if (*ptr == '\0') { return; }

		info->module_partition_name = dup_string(ptr);
		info->linetype = LINETYPE_IMPORT_DECLARATION;
		return;
	}
	// there is space between ':' and partition name
	else if (strcmp(tok, ":") == 0) {
		tok = strtok(NULL, "; ");
		if (tok == NULL) { return; }

		info->module_partition_name = dup_string(tok);
		info->linetype = LINETYPE_IMPORT_DECLARATION;
		return;
	}
	else {
		info->module_name = dup_string(tok);
		info->linetype = LINETYPE_IMPORT_DECLARATION;

		// NOTE: For now we ignore attr(optional)
		return;
	}
}


void read_line(const char* line, module_lineinfo_t* info)
{
	info->linetype = LINETYPE_INVALID_MODULE_SYNTAX;
	char* dup_line = dup_string(line);
	char* tok = strtok(dup_line, " ");

	// If the first token is "module" it is either a global
	// or private module fragment, or a module declaration.
	if (strcmp(tok, "module") == 0)
	{
		tok = strtok(NULL, " ");

		if (tok == NULL) { goto DONE; }
		else if (strcmp(tok, ";") == 0) {
			info->linetype = LINETYPE_GLOBAL_MODULE_FRAGMENT;
			goto DONE;
		}
		else if (strcmp(tok, ":") == 0) {
			info->linetype = LINETYPE_PRIVATE_MODULE_FRAGMENT;
			goto DONE;
		}

		// module declaration, first check module name
		read_module_declaration(info);
		goto DONE;
	}
	else if (strcmp(tok, "module;") == 0) {
		info->linetype = LINETYPE_GLOBAL_MODULE_FRAGMENT;
		goto DONE;
	}

	// If first word is "export" it can be an import declaration,
	// a module declaration, or an export declaration.
	else if(strcmp(tok, "export") == 0) {
		tok = strtok(NULL, " ");
		if (tok == NULL) { goto DONE; }

		// import declaration
		if (strcmp(tok, "import") == 0) {
			read_import_declaration(info);
			goto DONE;
		}
		// module declaration
		else if (strcmp(tok, "module") == 0) {
			read_module_declaration(info);
			goto DONE;
		}
		// export declaration.
		// At this point, no further parsing needs to be done!
		else {
			info->linetype = LINETYPE_EXPORT_DECLARATION;
			goto DONE;
		}
	}
	// import declaration, without exporting (but we don't care :) )
	else if (strcmp(tok, "import") == 0) {
		read_import_declaration(info);
		goto DONE;
	}
	else {
		info->linetype = LINETYPE_OTHER;
		goto DONE;
	}

 DONE:
	free(dup_line);
}


io_read_status_t read_module_unit_info(int num_lines,
									   const char** lines,
									   module_unit_t* module_unit)
{
	io_read_status_t ret = IO_READ_STATUS_SUCCESS;
	//module_linetype_t current_linetype;
	int has_error = 0;
	int should_stop = 0;
	int has_global_module_fragment_declared = 0;
	int has_module_unit_declared = 0;

	for (int i = 0; i < num_lines; i++)
	{
		if (has_error || should_stop) break;

		module_lineinfo_t info;
		init_lineinfo(&info);

		read_line(lines[i], &info);
		switch(info.linetype)
		{
		case LINETYPE_OTHER:
			printf("--> LINETYPE_OTHER\n");
			// TODO: Is this right?
			if (has_module_unit_declared) {
				should_stop = 1;
			}
			break;
		case LINETYPE_INVALID_MODULE_SYNTAX:
			printf("ERROR: Invalid module syntax\n");
			has_error = 1;
			break;
		case LINETYPE_MODULE_DECLARATION:
			if (has_module_unit_declared) {
				printf("ERROR: Module unit already declared\n");
				has_error = 1;
			} else {
				has_module_unit_declared = 1;
				printf("--> declare module: %s\n", info.module_name);
			}
			break;
		case LINETYPE_MODULE_PARTITION_DECLARATION:
			if (has_module_unit_declared) {
				printf("ERROR: Module unit already declared\n");
				has_error = 1;
			} else {
				has_module_unit_declared = 1;
				printf("--> declare module-partition: %s\n", info.module_partition_name);
			}
			break;
		case LINETYPE_EXPORT_DECLARATION:
			printf("--> export declaration\n");
			should_stop = 1;
			break;
		case LINETYPE_IMPORT_DECLARATION:
			printf("--> import declaration\n");
			break;
		case LINETYPE_GLOBAL_MODULE_FRAGMENT:
			if (has_global_module_fragment_declared) {
				printf("ERROR: Global module fragment already declared\n");
				has_error = 1;
			} else {
				has_global_module_fragment_declared = 1;
				printf("--> global module fragment\n");
			}
			break;
		case LINETYPE_PRIVATE_MODULE_FRAGMENT:
			printf("UNSUPPORTED: Encountered private module fragment\n");
			has_error = 1;
			break;
		default:
			printf("ERROR: Invalid linetype\n");
			has_error = 1;
			break;
		}

		free_lineinfo(&info);
	} // for(...)

	return ret;
}


static int is_file_module_unit(FILE* fp, char* line)
{
	size_t len;
	ssize_t nread;
	// NOTE: We can assume that a module line will never be longer than 1024.
	// This assumption is entirely valid for the module syntax.
	// Most, if not all, lines will be < 100 characters.
	while ((nread = getline(&line, &len, fp)) != -1)
	{
		printf("line: \"%s\"", line);
		char* tok = strtok(line, "; \n");
		printf("tok: \"%s\"\n", tok);
		if ((strcmp(tok, "module") == 0) || (strcmp(tok, "export") == 0)) return 1;
		else if (len <= 2 || tok[0] == '/') continue;
		else return 0;
	}
	return 0;
}


io_read_status_t parse_file(char* filename, module_unit_t* unit)
{
	io_read_status_t status = IO_READ_STATUS_NOT_MODULE;

	FILE* fp = fopen(filename, "r");
	if (!fp) return IO_READ_STATUS_FILE_NOT_EXISTS; // TODO: errno and strerr

	char* line = NULL; // reuse memory for the line (line below calls `getline`)
	if (is_file_module_unit(fp, line))
	{
		printf("~~~ Module unit\n");
		// rewind file pointer!
		// parse and fill out `unit` struct.
	}
	else
	{
		printf("~~~ Not module unit\n");
	}

	fclose(fp);
	return status;
}




const char* get_linetype_string(module_linetype_t lt)
{
	switch(lt)
	{
	case LINETYPE_OTHER:
		return "LINETYPE_OTHER";
	case LINETYPE_INVALID_MODULE_SYNTAX:
		return "LINETYPE_INVALID_MODULE_SYNTAX";
	case LINETYPE_MODULE_DECLARATION:
		return "LINETYPE_MODULE_DECLARATION";
	case LINETYPE_MODULE_PARTITION_DECLARATION:
		return "LINETYPE_MODULE_PARTITION_DECLARATION";
	case LINETYPE_EXPORT_DECLARATION:
		return "LINETYPE_EXPORT_DECLARATION";
	case LINETYPE_IMPORT_DECLARATION:
		return "LINETYPE_IMPORT_DECLARATION";
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
	case IO_READ_STATUS_FILE_NOT_EXISTS:
		return "IO_READ_STATUS_FILE_NOT_EXISTS";
	case IO_READ_STATUS_NOT_MODULE:
		return "IO_READ_STATUS_NOT_MODULE";
	case IO_READ_STATUS_MODULE:
		return "IO_READ_STATUS_MODULE";
	case IO_READ_STATUS_MODULE_PARTITION:
		return "IO_READ_STATUS_MODULE_PARTITION";
	case IO_READ_STATUS_SUCCESS:
		return "IO_READ_STATUS_SUCCESS";
	case IO_READ_STATUS_UNSUPPORTED_DECLARATION:
		return "IO_READ_STATUS_UNSUPPORTED_DECLARATION";
	default:
		return "ERROR: Unrecognized io_status";
	}
}
