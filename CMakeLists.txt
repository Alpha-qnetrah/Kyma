cmake_minimum_required(VERSION 3.25)
project(kyna VERSION 0.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(KYNA_BUILD_TESTS "Build Kyna tests" ON)
option(KYNA_ENABLE_SANITIZERS "Enable AddressSanitizer and UBSan" OFF)

function(configure_kyna_library target)
  target_include_directories(${target} PUBLIC include)
  target_compile_features(${target} PUBLIC cxx_std_23)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /permissive-)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
  endif()
  if(KYNA_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${target} PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(${target} PUBLIC -fsanitize=address,undefined)
  endif()
endfunction()

# The target graph is the architecture boundary. Each layer sees only the
# public headers of the layers to its left:
# source -> syntax -> modules -> semantics -> execution -> language.
add_library(kyna_source
    src/diagnostics/diagnostic_sink.cpp
    src/diagnostics/compiler_text_renderer.cpp
    src/diagnostics/json_diagnostic_renderer.cpp
    src/source/line_index.cpp
    src/source/source_manager.cpp
)
configure_kyna_library(kyna_source)

add_library(kyna_syntax
    src/lexing/legacy_tokenizer_adapter.cpp
    src/lexing/kyna_tokenizer.cpp
    src/lexing/keyword_catalog.cpp
    src/lexing/token_kind_description.cpp
    src/semantics/type_model.cpp
    src/parsing/declaration_and_statement_parser.cpp
    src/parsing/expression_parser.cpp
    src/parsing/kyna_module_parser.cpp
)
configure_kyna_library(kyna_syntax)
target_link_libraries(kyna_syntax PUBLIC kyna_source)

add_library(kyna_modules
    src/modules/filesystem_module_loader.cpp
    src/modules/module_graph_builder.cpp
    src/modules/module_cycle_detector.cpp
)
configure_kyna_library(kyna_modules)
target_link_libraries(kyna_modules PUBLIC kyna_syntax)

add_library(kyna_semantics
    src/semantic/analyzer.cpp
    src/semantic/type_checker.cpp
    src/semantics/best_practice_checker.cpp
    src/semantics/modifier_rules.cpp
    src/semantics/interface_catalog.cpp
    src/semantics/program_validation.cpp
    src/semantics/module_analyzer.cpp
)
configure_kyna_library(kyna_semantics)
target_link_libraries(kyna_semantics PUBLIC kyna_modules)

add_library(kyna_execution
    src/execution/module_program_executor.cpp
    src/execution/production_runtime_capabilities.cpp
    src/runtime/runtime_value.cpp
    src/runtime/lexical_environment.cpp
    src/runtime/function_value.cpp
    src/memory/managed_heap.cpp
    src/stdlib/standard_library_catalog.cpp
    src/stdlib/json_value_codec.cpp
    src/execution/statement_executor.cpp
    src/execution/expression_evaluator.cpp
)
configure_kyna_library(kyna_execution)
target_link_libraries(kyna_execution PUBLIC kyna_semantics)

add_library(kyna_language
    src/language/language_session.cpp
)
configure_kyna_library(kyna_language)
target_link_libraries(kyna_language PUBLIC kyna_execution)

# Pre-v0.3 build compatibility. New embedders should link kyna_language or a
# narrower domain target instead of the aggregate.
add_library(kyna_lib INTERFACE)
target_link_libraries(kyna_lib INTERFACE kyna_language)

add_executable(kyna
    src/cli/kyna_main.cpp
    src/cli/cli_argument_parser.cpp
    src/cli/command_dispatcher.cpp
    src/cli/source_run_command.cpp
    src/cli/source_check_command.cpp
    src/cli/repl_command.cpp
    src/cli/token_dump_command.cpp
    src/cli/syntax_dump_command.cpp
)
target_link_libraries(kyna PRIVATE kyna_lib)

if(KYNA_BUILD_TESTS)
  enable_testing()
  function(add_kyna_test name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE kyna_lib)
    add_test(NAME ${name} COMMAND ${name})
  endfunction()
  add_kyna_test(kyna_tests tests/test_kyna.cpp)
  add_kyna_test(kyna_lexer_tests tests/lexer/test_lexer.cpp)
  add_kyna_test(kyna_parser_tests tests/parser/test_parser.cpp)
  add_kyna_test(kyna_semantic_tests tests/semantic/test_semantic.cpp)
  add_kyna_test(kyna_runtime_tests tests/runtime/test_runtime.cpp)
  add_kyna_test(kyna_stdlib_tests tests/stdlib/test_stdlib.cpp)
  add_kyna_test(kyna_v03_language_tests tests/v03/test_language_session.cpp)
endif()

include(GNUInstallDirs)
install(TARGETS kyna RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES README.md LICENSE DESTINATION share/kyna)
install(DIRECTORY docs DESTINATION share/kyna)
install(DIRECTORY examples DESTINATION share/kyna
        FILES_MATCHING PATTERN "*.kyna" PATTERN "*.ky"
        PATTERN "weather-output" EXCLUDE)

set(CPACK_PACKAGE_NAME "kyna")
set(CPACK_PACKAGE_VENDOR "kyna-lang")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Kyna programming language and CLI")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Kyna")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_GENERATOR "TGZ;ZIP")
set(CPACK_PACKAGE_FILE_NAME "kyna-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
include(CPack)
