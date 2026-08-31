kyna_add_module(kyna_hir SOURCES
  src/syntax_lowering.cpp
  src/hir_renderer.cpp
  PUBLIC_DEPENDENCIES kyna_diagnostics kyna_syntax)
