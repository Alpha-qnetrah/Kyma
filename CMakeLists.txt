cmake_minimum_required(VERSION 3.25)
project(kyma VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(KYMA_BUILD_TESTS "Build Kyma tests" ON)
option(KYMA_ENABLE_SANITIZERS "Enable AddressSanitizer and UBSan" OFF)

add_library(kyma_lib
    src/lexer/lexer.cpp
    src/lexer/keywords.cpp
    src/lexer/words.cpp
    src/types/types.cpp
    src/parser/parser.cpp
    src/parser/expressions.cpp
    src/semantic/analyzer.cpp
    src/semantic/type_checker.cpp
    src/semantic/behavior.cpp
    src/semantic/interfaces.cpp
    src/semantic/validation.cpp
    src/runtime/values.cpp
    src/runtime/environment.cpp
    src/runtime/functions.cpp
    src/runtime/gc.cpp
    src/stdlib/stdlib.cpp
    src/interpreter/interpreter.cpp
    src/interpreter/expressions.cpp
)
target_include_directories(kyma_lib PUBLIC include)
target_compile_features(kyma_lib PUBLIC cxx_std_23)
target_compile_options(kyma_lib PRIVATE -Wall -Wextra -Wpedantic)
if(KYMA_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(kyma_lib PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
  target_link_options(kyma_lib PUBLIC -fsanitize=address,undefined)
endif()

add_executable(kyma src/cli/main.cpp)
target_link_libraries(kyma PRIVATE kyma_lib)

if(KYMA_BUILD_TESTS)
  enable_testing()
  function(add_kyma_test name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE kyma_lib)
    add_test(NAME ${name} COMMAND ${name})
  endfunction()
  add_kyma_test(kyma_tests tests/test_kyma.cpp)
  add_kyma_test(kyma_lexer_tests tests/lexer/test_lexer.cpp)
  add_kyma_test(kyma_parser_tests tests/parser/test_parser.cpp)
  add_kyma_test(kyma_semantic_tests tests/semantic/test_semantic.cpp)
  add_kyma_test(kyma_runtime_tests tests/runtime/test_runtime.cpp)
  add_kyma_test(kyma_stdlib_tests tests/stdlib/test_stdlib.cpp)
endif()

include(GNUInstallDirs)
install(TARGETS kyma RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
