kyna_add_module(kyna_parsing SOURCES
  src/declaration_and_statement_parser.cpp
  src/expression_parser.cpp
  src/kyna_module_parser.cpp
  PUBLIC_DEPENDENCIES kyna_syntax kyna_lexing kyna_diagnostics)
