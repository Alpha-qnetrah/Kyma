kyna_add_module(kyna_host SOURCES
  src/production_runtime_capabilities.cpp
  src/postgres_database.cpp
  PRIVATE_DEPENDENCIES CURL::libcurl)

if(PostgreSQL_FOUND)
  target_compile_definitions(kyna_host PRIVATE KYNA_HAS_POSTGRESQL=1)
  target_link_libraries(kyna_host PRIVATE PostgreSQL::PostgreSQL)
endif()
