kyna_add_module(kyna_diagnostics SOURCES
  src/diagnostic_sink.cpp
  src/compiler_text_renderer.cpp
  src/json_diagnostic_renderer.cpp
  PUBLIC_DEPENDENCIES kyna_source)
