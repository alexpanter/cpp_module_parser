# cpp_module_parser #

Experimentation with creating build scripts for GCC with C++20 modules

## Motivation ##

C++-20 introduced a new feature called "modules".
This is likely the most drastic change to the langauge since its inception,
and is a huge effort in the direction of eliminating the preprocessing stage.

Likely, the preprocessing stage will never truly be removed as it does carry
out a very important function in the overall building process, but with modules
we now have an alternative to the error-prone header-inclusion method of
building C++ projects.

## Challenges ##

The header-inclusion process is embarrasingly parallel, meaning that translation
units can be built in arbitrary order no matter their interdependencies.

This is, unfortunately, not possible with modules. Modules are essentially a
binary representation of a parsed abstract syntax tree representing a translation
unit, that is a source file preprocessed, lexed, and parsed, but not assembled.
And as such, they are strictly dependent upon already built module units (BMI's)
for each module unit that they import, in order to generate the syntax tree.

Due to this requirement, creating a build system for modules requires a series
of steps:

1) parsing all references source files in the current project, read their module
   declarations and dependencies (always located at the top of the file).
2) build a dependency graph for all module units in the project, where a source
   file is a module unit only if it declares itself a module unit.
3) generate a sequence of build commands, where sources files with no module
   dependencies are built first, and then recursively their parents in the graph.

## What does this program do ##

NOTE: This is still work in progress!

The intent is to provide a backend for
[premake](https://github.com/premake/premake-core/), for generating build scripts
for the GNU G++-11 compiler toolset.

## License ##

This software is released under a permissive license but without warranties
(see LICENSE.md for details).
