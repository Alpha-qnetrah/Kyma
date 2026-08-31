kyna_add_module(kyna_embedding SOURCES src/language_session.cpp
  PUBLIC_DEPENDENCIES kyna_standard_library kyna_resolution kyna_typecheck kyna_hir kyna_mir
                      kyna_bytecode kyna_vm)
