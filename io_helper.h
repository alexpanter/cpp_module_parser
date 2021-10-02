#ifndef IO_HELPER_H
#define IO_HELPER_H

#include <string.h>

/*
 * According to cppreference:
 * https://en.cppreference.com/w/cpp/language/modules
 * the syntax for modules are as following:
 * 1) export(opt) module module-name module-partition(opt) attr(opt) ;
 * 2) export declaration
 * 3) export { declaration-seq(opt) }
 * 4) export(opt) import module-name attr(opt) ;
 * 5) export(opt) import module-partition attr(opt) ;
 * 6) export(opt) import header-name attr(opt) ;
 * 7) module ;
 * 8) module : private ;
 *
 * These are categorized into 5 groups:
 * 1) Module declaration.
 *    Declares that the current translation unit is a module unit.
 * 2,3) Export declaration.
 *    Export all namespace-scope declarations in 'declaration'
 *    or 'declaration-seq'.
 * 4,5,6) Import declaration.
 *    Import a module unit/module partition/header unit.
 * 7) Stars a global module fragment.
 * 8) Starts a private module fragment.
 *
 * Rules for line-termination are such that:
 * 1,4,5,6,7,8) _must_ all appear in one line, ie. their
 * tokens must not be divided into multiple lines.
 * 2,3) the declaration may appear on another line.
 *
 * NOTE: We actually don't care about all the C++-rules as
 * the compiler will check that for us; we just wish to
 * determine the module inter-dependencies.
 * So we solely look for "export", "import", and
 * "export import".
 *
 * All import declarations (4,5,6) must be grouped after the
 * module declaration and before all other declarations.
 * So if we see an export declaration we are done parsing
 * the file.
 */
typedef enum {
      LINETYPE_OTHER,
      LINETYPE_INVALID_MODULE_SYNTAX,
      LINETYPE_MODULE_DECLARATION,
      LINETYPE_MODULE_PARTITION_DECLARATION,
      LINETYPE_EXPORT_DECLARATION,
      LINETYPE_IMPORT_DECLARATION,
      LINETYPE_GLOBAL_MODULE_FRAGMENT,
      LINETYPE_PRIVATE_MODULE_FRAGMENT
} module_linetype_t;

typedef struct {
  module_linetype_t linetype;
  char* module_name;
  char* module_partition_name;
  char* header_name;
  // TODO: attr(optional)
} module_lineinfo_t;

/*
 * Read a line and report type of C++ line.
 */
void read_line(const char* line,
	       module_lineinfo_t* info)
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
      tok = strtok(NULL, "; ");
      if (tok == NULL) { return; }
      info->module_name = strdup(tok);

      tok = strtok(NULL, " ");
      if (tok == NULL) { return; }
      else if (strcmp(tok, ";") == 0) {
	info->linetype = LINETYPE_MODULE_DECLARATION;
	return;
      }
      // check partition name
      else if (strcmp(tok, ":") == 0) {
	tok = strtok(NULL, " ");
	if (tok == NULL) { return; }

	info->module_partition_name = strdup(tok);
	// TODO: This would be the place to check attr(optional)
	// TODO: Might also report invalid syntax if semicolon is missing
	info->linetype = LINETYPE_MODULE_PARTITION_DECLARATION;
	return;
      }
      else if (tok[0] == ':') {
	// We need to split ':' from the partition name
	char* ptr = tok[1];
	if (*ptr == '\0') { return; }

	info->module_partition_name = strdup(ptr);
	info->linetype = LINETYPE_MODULE_PARTITION_DECLARATION;
	return;
      }
      // TODO: For now we ignore attr(optional)
      else { return; }
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

    if (strcmp(tok, "import") == 0) {
      tok = strtok(NULL, " ");
      if (tok == NULL) { return; }
    }
  }

}



#endif // IO_HELPER_H
