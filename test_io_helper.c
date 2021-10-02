#include "io_helper.h"
#include <stdio.h>

#define TEST_PRINT_ALL

void testline(const char* line)
{
  module_lineinfo_t info;
  init_lineinfo(&info);
  read_line(strdup(line), &info);
#if defined(TEST_PRINT_ALL)
  printf("\"%s\"\n", line);
  printf("\tlinetype: %s\n", get_linetype_string(info.linetype));
  printf("\tmodule-name: \"%s\"\n", info.module_name);
  printf("\tpartition-name: \"%s\"\n", info.module_partition_name);
  printf("\theader-name: \"%s\"\n", info.header_name);
#endif
}


int main()
{
  testline("module;");
  testline("module ;");
  testline("export module banana;");
  testline("export module banana ;");
  testline("import mango;");
  testline("import mango ;");
  testline("export import mango");
  testline("export module orange:blood_orange;");
  testline("export module orange :blood_orange;");
  testline("export import :blood_orange");
  return 0;
}
