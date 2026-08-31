kyna_add_module(kyna_resolution SOURCES
  src/filesystem_module_loader.cpp
  src/module_graph_builder.cpp
  src/module_cycle_detector.cpp
  PUBLIC_DEPENDENCIES kyna_parsing kyna_diagnostics)
