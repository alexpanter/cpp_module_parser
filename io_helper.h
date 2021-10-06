#ifndef IO_HELPER_H
#define IO_HELPER_H

#include <string.h>
#include "module_unit.h"


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
typedef enum
{
	LINETYPE_UNDETERMINED,
	LINETYPE_OTHER,
	LINETYPE_EMPTY,
	LINETYPE_PREPROCESSING_DIRECTIVE,
	LINETYPE_INVALID_MODULE_SYNTAX,
	LINETYPE_MODULE_DECLARATION,
	LINETYPE_MODULE_PARTITION_DECLARATION,
	LINETYPE_EXPORT_DECLARATION,
	LINETYPE_IMPORT_MODULE,
	LINETYPE_IMPORT_PARTITION,
	LINETYPE_IMPORT_HEADER,
	LINETYPE_GLOBAL_MODULE_FRAGMENT,
	LINETYPE_PRIVATE_MODULE_FRAGMENT
} module_linetype_t;

typedef struct
{
	module_linetype_t linetype;
	char* module_name;
	char* partition_name;
	char* header_name;
	// TODO: attr(optional)
} module_lineinfo_t;

typedef enum
{
	IO_READ_STATUS_UNDETERMINED,
	IO_READ_STATUS_FILE_NOT_EXISTS,
	IO_READ_STATUS_INTERNAL_ERROR,

	IO_READ_STATUS_NOT_MODULE,
	// TODO: Does gcc care about module/partition ?

	// The only two valid outputs:
	IO_READ_STATUS_MODULE,
	IO_READ_STATUS_MODULE_PARTITION,

	IO_READ_STATUS_INVALID_MODULE_SYNTAX,
	IO_READ_STATUS_DUPLICATE_DEFINITION,
	IO_READ_STATUS_UNSUPPORTED_DECLARATION
} io_read_status_t;


/*
 * Initialize a `module_lineinfo_t` struct.
 */
void init_lineinfo(module_lineinfo_t* info);

/*
 * Free a `module_lineinfo_t` struct.
 */
void free_lineinfo(module_lineinfo_t* info);

/*
 * Read a line and report type of C++ line.
 */
void read_line(char* line, module_lineinfo_t* info);

/*
 * Open a C++ source file and attempt to parse it as a module unit.
 * Returns an `io_read_status_t` enum and fills out the referenced
 * `module_unit_t` struct containing info about the module unit
 * described in the file.
 */
io_read_status_t parse_file(char* filename, module_unit_t* unit);


/*
 * Console output helper functions.
 */
const char* get_linetype_string(module_linetype_t lt);
const char* get_io_status_string(io_read_status_t rs);

#endif // IO_HELPER_H
