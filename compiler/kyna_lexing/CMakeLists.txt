kyna_add_module(kyna_lexing SOURCES
  src/legacy_tokenizer_adapter.cpp
  src/kyna_tokenizer.cpp
  src/keyword_catalog.cpp
  src/token_kind_description.cpp
  PUBLIC_DEPENDENCIES kyna_source kyna_diagnostics)
