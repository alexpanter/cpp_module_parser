# cpp_module_parser [cmop] #

A tool for generating build scripts for C++20 projects that use modules.

The intent is to provide a backend for
[premake](https://github.com/premake/premake-core/), for generating build
scripts.
For the sake of integrating with the existing codebase of premake, and
potentially other build script generators, a choice was made to write this
software using C.

Though intended at targeting GNU Makefile and the GCC toolset (>= g++-11),
support for additional compiler vendors and platforms may be added.

## How to install ##

This repository may be cloned after which the terminal should be positioned
inside the root folder of the repository. Then build with

```
make all
```

This will generate two executables inside the "bin/" folder:

```
$ ls bin/
cmop test_io_reader [object file]...
```

To run the tests, write `bin/test_io_reader`. If the test program is run
from within the "bin/" folder then it cannot locate the test files. The main
executable "cmop" may be run like any other program, granted that it has the
necessary privileges.

Currently there are no alternate build tools supported, though a premake
script may be added at some point. I have no intentions of creating a CMakeLists.txt
file though such contribution is welcome.

## Motivation ##

C++-20 introduced a new feature called "modules".
This is easily one of the most important additions to the language since its
inception, and is a huge effort in the direction of eliminating the preprocessing
stage.

Likely, the preprocessing stage will never truly be removed as it does carry
out very important functionality in the overall building process, but with modules
we now have an alternative to the error-prone header-inclusion approach of
building C++ projects.

## What are modules ##

With C++20 we have 5 different kinds of "file type":

* Regular header file
* Header unit
* Regular source file
* Module unit
* Module partition unit

Regular header files and source files have not changed from earlier versions of C++.
A header unit is the same as a header file, except that it has been compiled into a
header unit (BMI) and may be `import`'ed by all other file types. This is the
difference:

```cpp
#include <iostream>       // regular header file -> preprocessor copy/paste
import <iostream>;        // header unit -> reference to precompiled header file
```

Module units are source files that declare a module with either `export module name;`
or `module name;` at the top. If, however, the `export` keyword is not provided
the module may not be imported by other units of code and thus carries no practical
meaning. Module units can be imported by all file types.

Module partition units (or module partitions) may only be imported by the module to
which they are a partition of. A module partition is declared with either
`export module name: part;` or `module name : part;`, and is the a partition of
module `name`. Without the `export` keyword its definitions will not be visible
outside its containing module.

## Challenges ##

The header-inclusion approach is inherently parallel, meaning that translation units
can be built in arbitrary order no matter their interdependencies.

This is, unfortunately, not easily possible with modules. Modules are essentially a
binary representation of an abstract syntax tree, representing a translation unit:
That is a source file preprocessed, lexed, and parsed, but not assembled.

Modules seek to remove the distinction between header files providing declarations,
and source files providing definitions, eliminating a great source of confusion and
errors in building and linking C++ projects.

Therefore we can now have C++ projects with source files only. This does though
introduce the dependency on precompiled binary module interfaces (BMI's) whenever
those source files import other modules. Without these BMI's the compiler cannot
generate the syntax tree.

Due to this requirement, creating a build tool for modules requires a series of
additional steps:

1) read all referenced source files in the project and interpret their module
   declarations and dependencies (always located at the top of the file).
   Even though a source file is not required to declare itself a module unit,
   it may still import other modules.
2) build a dependency graph for all module units in the project.
3) generate a sequence of build commands, where sources files with no module
   dependencies are built first, and then recursively their parents in the graph.

### Building order matters ###

We cannot simply decide to build header units first, then module partitions
and then modules. Reason: They may all import modules! This is a valid C++-20
header file:

```cpp
// some_header.hpp
#pragma once

#include <iostream>;
import my_module;

#define MEANING_OF_LIFE 42

void print_OS() {
#ifdef _WIN32
    std::cout << "Doing Windows stuff...\n";
#else
    std::cout << "Not on Windows!\n";
#endif
}
```

Similarly, module partitions may also import modules. Module partitiones may also
import other partitioned from the same named module. C++ places the following
restrictions on the usage of modules:

* headers may not declare a module
* a module or module partition may not import itself, since that would be
  impossible for the compiler to resolve.
* module partitions may not be imported by other modules or their partitions.
* there may not be circular dependencies between modules or module partitions.
* module partitions may import their own module; however this is not possible in
  practice since it will always lead to a circular dependency when the module
  needs access to the definitions of that partition. So for practicality
  we say that it's not possible - modules may import their partitions, never the
  other way around!

Meanwhile, these usages are allowed:

* headers may import modules.
* modules may import other modules.
* module partitions may import other modules.
* module partitions may import other partitions from the same module.
* source files that do not declare a module may import modules or header units.

The last one is useful for building executables. While the main file can still
declare and export a module, that usage is limited.

When building libraries, exporting a module (or modules) is necessary. A library
may additionally also provide a main header file, which the preprocessor of
consuming projects can use to copy/paste module imports for ease of use.

### Summary ###

For reasons discussed above, employing a strategy such as building module
partitions first will not work. Also, neither can we compile header units first.
Since we cannot make any assumptions on build orders, all code files, header or
not, must be given the same treatment. The only exception to this rule applies when
importing headers from the standard library as header units. Though this presents
a problem: When importing a header with `import <header>;` that header may be
located in some permutation/extension of "/usr/include/...", or it may be from
a local directory provided as include path on the command line. Besides from
maintaining a list of headers located in standard header locations, there is
no easy way to distinguish between these.

But we do not need to parse standard library headers to determine their
dependencies. First of all, these headers may change with C++23 when the
entire standard library is expected to be "modularized". Secondly, if these
header files are not provided to this build tool they will not be parsed, and
we will not need to worry about their internal module dependencies (this should
be addressed by the implementors of the standard library and the C++ Committee,
not by implementors of this or similar tools).


## What does this program do ##

This project is still work in progress, and an interface has yet to be declared.

## Preliminary example build ##

Currently, the program can be run with a test file (assuming Linux bash terminal):

```
echo example_files/large_module_unit.cpp | bin/cmop --output=terminal --debug-print
```

The testfile "example_files/large_module_unit.cpp" looks like this:

```cpp
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

```

The tool will generate the following output:

```
program_args:
--> compiler=(null)
--> compiler_flags=(null)
--> file=(null)
--> output_format=OUTPUT_FORMAT_TERMINAL
--> debug_print=<true>
file: example_files/large_module_unit.cpp
--> module-type: MODULE_TYPE_PARTITION
--> module-name: "large_module.unit"
--> partition-name: "partition_1"
--> dependencies:
----> [MODULE] "large_module.common_unit"
----> [MODULE] "large_module.other_unit"
----> [PARTITION] "partition_3"
----> [PARTITION] "partition_2"
----> [HEADER] "some_header_2.hpp"
----> [HEADER] "some_header_1.hpp"
----> [HEADER] "vector"
----> [HEADER] "thread"
----> [HEADER] "iostream"
```

TODO: Next step is to create a dependency graph and parse that to generate build commands.


## License ##

This software is released under the BSD-2-Clause License, to provide maximal
flexibility of use and to promote software freedom.
