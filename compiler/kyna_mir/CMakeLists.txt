kyna_add_module(kyna_mir SOURCES
  src/hir_lowering.cpp
  src/mir_verifier.cpp
  src/mir_renderer.cpp
  PUBLIC_DEPENDENCIES kyna_diagnostics kyna_hir)
