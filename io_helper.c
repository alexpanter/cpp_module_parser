#include "io_helper.h"
#include <stdio.h>
#include <stdlib.h>

static char* dup_string(char* str)
{
  size_t len = strlen(str);
  char* mem = malloc(len + 1);
  strcpy(mem, str);
  return mem;
}


static char* dup_string_n(char* str, size_t n)
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
        printf("info->module_name: \"%s\"\n", info->module_name);
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
  printf("tok: %s\n", tok);
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


void read_line(char* line, module_lineinfo_t* info)
{
  info->linetype = LINETYPE_INVALID_MODULE_SYNTAX;
  char* tok = strtok(line, " ");

  // If the first token is "module" it is either a global
  // or private module fragment, or a module declaration.
  if (strcmp(tok, "module") == 0)
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
  }
  else {
    info->linetype = LINETYPE_OTHER;
    return;
  }
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
