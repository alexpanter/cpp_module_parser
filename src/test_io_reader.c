#include "io_reader.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int num_failed = 0;

typedef struct
{
	read_status_t read_status;
	module_type_t module_type;

	const char* module_name;
	const char* partition_name;

	char** module_deplist;
	char** partition_deplist;
	char** header_deplist;

	// TODO: If I want to be really cool:
	//unsigned long line_num;
} expected_t;

#define SAFE_STR(s) ((s == NULL) ? "" : s)

int compare_str(const char* a, const char* b) {
	if (strcmp(SAFE_STR(a), SAFE_STR(b)) != 0) {
		printf("--> expected: ");
		printf((a == NULL) ? "%s": "\"%s\"", a);
		printf(", actual: ");
		printf((b == NULL) ? "%s": "\"%s\"", b);
		printf("\n");
		return 1;
	}
	return 0;
}

void testfile(char* filename, expected_t* expected)
{
	int errors = 0;
	module_unit_t unit;
	module_unit_init(&unit);
	read_status_t status = read_file(filename, &unit);

	printf("> %s:\n", filename);
	if (expected->read_status != status) {
		printf("--> expected: %s, actual: %s\n",
			   get_read_status_string(expected->read_status),
			   get_read_status_string(status));
		errors++;
	}
	printf("--> lines read: %li\n", unit.line_num);
	if (expected->module_type != unit.module_type) {
		printf("--> expected: %s, actual: %s\n",
			   get_module_type_string(expected->module_type),
			   get_module_type_string(unit.module_type));
		errors++;
	}
	errors += compare_str(expected->module_name, unit.module_name);
	errors += compare_str(expected->partition_name, unit.partition_name);

	if (expected->module_deplist != NULL) {
		int i = 0;
		char* str = expected->module_deplist[i];
		while(str != NULL) {
			if (!module_unit_imports_module(&unit, str)) {
				printf("--> expected module import \"%s\"\n", str);
				errors++;
			}
			str = expected->module_deplist[++i];
		}
	}

	if (expected->partition_deplist != NULL) {
		int i = 0;
		char* str = expected->partition_deplist[i];
		while(str != NULL) {
			if (!module_unit_imports_partition(&unit, str)) {
				printf("--> expected partition import \"%s\"\n", str);
				errors++;
			}
			str = expected->partition_deplist[++i];
		}
	}


	module_unit_free(&unit);
	if (errors == 0) {
		printf("--> OK.\n");
	}
	num_failed += errors;
	printf("\n");
}



void test_non_existent_file()
{
	expected_t exp = {
		READ_STATUS_FILE_NOT_EXISTS, MODULE_TYPE_UNDETERMINED,
		NULL, NULL,
		NULL, NULL, NULL
	};
	testfile("example_files/non_existent_file.cpp", &exp);
}

void test_ignore()
{
	char** module_deplist = malloc(sizeof(char*) * 3);
	module_deplist[0] = "nonimportant";
	module_deplist[1] = "dontcare";
	module_deplist[2] = NULL;

	expected_t exp = {
		READ_STATUS_MODULE, MODULE_TYPE_MODULE,
		"ignore", NULL,
		module_deplist, NULL, NULL
	};
	testfile("example_files/ignore.cpp", &exp);
}

void test_export_module_computer()
{
	char** partition_deplist = malloc(sizeof(char*) * 2);
	partition_deplist[0] = "cpu";
	partition_deplist[1] = NULL;

	expected_t exp = {
		READ_STATUS_MODULE, MODULE_TYPE_MODULE,
		"computer", NULL,
		NULL, partition_deplist, NULL
	};
	testfile("example_files/export_module_computer.cpp", &exp);
}
void test_export_module_computer_cpu()
{
	char** module_deplist = malloc(sizeof(char*) * 2);
	module_deplist[0] = "hardware";
	module_deplist[1] = NULL;

	expected_t exp = {
		READ_STATUS_MODULE, MODULE_TYPE_PARTITION,
		"computer", "cpu",
		module_deplist, NULL, NULL
	};
	testfile("example_files/export_module_computer-cpu.cpp", &exp);
}

void test_multiline_comment()
{
	expected_t exp = {
		READ_STATUS_MODULE, MODULE_TYPE_MODULE,
		"test", NULL,
		NULL, NULL, NULL
	};
	testfile("example_files/comments_test/multiline_comment_1.cpp", &exp);
}

void test_large_module_unit()
{
	expected_t exp = {
		READ_STATUS_MODULE, MODULE_TYPE_PARTITION,
		"large_module.unit", "partition_1",
		NULL, NULL, NULL
	};
	testfile("example_files/large_module_unit.cpp", &exp);
}


void is_module_test_negative_1()
{
	expected_t exp = {
		READ_STATUS_NOT_MODULE, MODULE_TYPE_UNDETERMINED,
		NULL, NULL,
		NULL, NULL, NULL
	};
	testfile("example_files/is_module_test/negative_1.cpp", &exp);
}


void is_module_test_invalid_1()
{
	expected_t exp = {
		READ_STATUS_INVALID_SYNTAX, MODULE_TYPE_INVALID,
		NULL, NULL,
		NULL, NULL, NULL
	};
	testfile("example_files/is_module_test/invalid_1.cpp", &exp);
}

void is_module_test_invalid_2()
{
	expected_t exp = {
		READ_STATUS_INVALID_SYNTAX, MODULE_TYPE_INVALID,
		NULL, NULL,
		NULL, NULL, NULL
	};
	testfile("example_files/is_module_test/invalid_2.cpp", &exp);
}

void is_module_test_invalid_3()
{
	expected_t exp = {
		READ_STATUS_INVALID_SYNTAX, MODULE_TYPE_INVALID,
		NULL, NULL,
		NULL, NULL, NULL
	};
	testfile("example_files/is_module_test/invalid_3.cpp", &exp);
}

int main()
{
	test_non_existent_file();
	test_ignore();
	test_export_module_computer();
	test_export_module_computer_cpu();
	test_multiline_comment();
	test_large_module_unit();
	is_module_test_negative_1();
	is_module_test_invalid_1();
	is_module_test_invalid_2();
	is_module_test_invalid_3();

	if (num_failed == 0)
		printf("All tests passed.\n");
	else
		printf("%i test(s) failed!\n", num_failed);
	return 0;
}
