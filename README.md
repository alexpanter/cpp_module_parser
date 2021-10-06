# cpp_module_parser #

A tool for generating build scripts for C++20 projects that use modules.

The intent is to provide a backend for
[premake](https://github.com/premake/premake-core/), for generating build
scripts.
For the sake of integrating with the existing codebase of premake, and
potentially other build script generators, a choice was made to write this
software using C.

Though intended at targeting GNU Makefile and the GCC toolset (>= g++-11),
support for additional compiler vendors and platforms may be added.

## Motivation ##

C++-20 introduced a new feature called "modules".
This is easily one of the most important additions to the language since its
inception, and is a huge effort in the direction of eliminating the preprocessing
stage.

Likely, the preprocessing stage will never truly be removed as it does carry
out very important functionality in the overall building process, but with modules
we now have an alternative to the error-prone header-inclusion approach of
building C++ projects.

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

## What does this program do ##

This project is still work in progress, and an interface has yet to be declared.


## License ##

This software is released under the BSD-2-Clause License, to provide maximal
flexibility of use and to promote software freedom.
