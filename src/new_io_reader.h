#ifndef NEW_IO_READER_H
#define NEW_IO_READER_H

typedef enum
{
	// error status
	READ_STATUS_FILE_NOT_EXISTS,
	READ_STATUS_INVALID_SYNTAX,

	// ok status
	READ_STATUS_NOT_MODULE,
	READ_STATUS_MODULE,
	READ_STATUS_MODULE_PARTITION
} read_status_t;


/*
 * Read through file and report its type.
 * If the file declares a module unit, and its module syntax
 * is valid, fill out the module_unit_t struct pointed to by
 * `unit` such that it contains information about the module
 * and its dependencies.
 */
read_status_t read_file(char* filename, module_unit_t* unit);


#endif // NEW_IO_READER_H
