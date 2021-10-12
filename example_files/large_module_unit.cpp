/*
 * This is an example of a module partition unit.
 *
 * The first section of the file is know as the
 * "global module fragment", and can be recognized by the
 * `module;` declaration.
 *
 * Inside that section only preprocessor directives may appear.
 *
 * The global module fragment ends at the module declaration,
 * which begins with the keywords `export module`. The colon :
 * signifies this module unit to be a module partition.
 *
 * Module processing ends with the first `export` declaration,
 * which in this example is followed by a scope in which all
 * definitions are considered part of the interface for this
 * module unit.
 */
module;

#ifndef NDEBUG
#define NDEBUG
#endif

#include <assert.h>
#include <sys/socket.h>

#define MAX(a, b) ((a > b) ? a : b)
#define SPECIAL_VALUE 42

#define MACRO_FUNCTION(x) ((x > 42)	\
                           ? "x is larger than 42"	\
                           : "x is not larger than 42")

export module large_module.unit : partition_1;

import <iostream>;
import <thread>;
import <vector>;

import "some_header_1.hpp";
import "some_header_2.hpp";

import large_module.other_unit;
import large_module.common_unit;

import :partition_2;
import :partition_3;

export
{
	int add(int a, int b) { return a + b; }
}
