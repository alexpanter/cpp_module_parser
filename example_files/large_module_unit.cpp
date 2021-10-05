module;

#ifndef NDEBUG
#define NDEBUG
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define MAX(a, b) ((a > b) ? a : b)
#define SPECIAL_VALUE 42

#define MACRO_FUNCTION(x) ((x > 42)	\
                           ? "x is larger than 42"	\
                           : "x is not larger than 42")

export module large_module.unit : partition_1;

export
{
	int add(int a, int b) { return a + b; }
}
