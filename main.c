#include "mockup_files.h"

const char* cyclic_dependency() {
  return "Cyclic module dependency detected. Aborting build script!";
}

const char* invalid_module_syntax() {
  return "Invalid module syntax. Failed to generate build script!";
}


int main()
{
  return 0;
}
