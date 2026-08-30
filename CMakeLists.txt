cmake_minimum_required(VERSION 3.25)
project(kyma VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(KYMA_BUILD_TESTS "Build Kyma tests" ON)
option(KYMA_ENABLE_SANITIZERS "Enable AddressSanitizer and UBSan" OFF)

add_library(kyma_lib
    src/lexer/lexer.cpp
    src/parser/parser.cpp
    src/semantic/analyzer.cpp
    src/runtime/runtime.cpp
    src/interpreter/interpreter.cpp
)
target_include_directories(kyma_lib PUBLIC include)
target_compile_features(kyma_lib PUBLIC cxx_std_20)
target_compile_options(kyma_lib PRIVATE -Wall -Wextra -Wpedantic)
if(KYMA_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(kyma_lib PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
  target_link_options(kyma_lib PUBLIC -fsanitize=address,undefined)
endif()

add_executable(kyma src/cli/main.cpp)
target_link_libraries(kyma PRIVATE kyma_lib)

if(KYMA_BUILD_TESTS)
  enable_testing()
  add_executable(kyma_tests tests/test_kyma.cpp)
  target_link_libraries(kyma_tests PRIVATE kyma_lib)
  add_test(NAME kyma_tests COMMAND kyma_tests)
endif()

include(GNUInstallDirs)
install(TARGETS kyma RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
