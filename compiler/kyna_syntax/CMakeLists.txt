add_library(kyna_syntax INTERFACE)
target_include_directories(kyna_syntax INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(kyna_syntax INTERFACE kyna_source kyna_lexing kyna_types)
