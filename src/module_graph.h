#ifndef MODULE_GRAPH_H
#define MODULE_GRAPH_H

typedef enum {
  MODULE_STATUS_UNCOMPILED,
  MODULE_STATUS_COMPILED
} module_compile_status_t;

typedef struct {
  // TODO: List of dependencies
  char* module_name;
  module_compile_status_t compile_status;
} module_unit_info_t;

typedef struct {
  module_unit_info_t* leaves;
} module_dependency_graph;


#endif // MODULE_GRAPH_H
