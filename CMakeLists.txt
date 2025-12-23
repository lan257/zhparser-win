cmake_minimum_required(VERSION 3.20)
project(zhparser-windows C)

# User should pass PG_CONFIG or it will try to find in PATH
if(NOT DEFINED PG_CONFIG)
  find_program(PG_CONFIG_EXECUTABLE pg_config PATHS ENV PATH HINTS "D:/pg17/bin")
  if(PG_CONFIG_EXECUTABLE)
    set(PG_CONFIG "${PG_CONFIG_EXECUTABLE}" CACHE FILEPATH "Path to pg_config.exe")
  else()
    message(FATAL_ERROR "PG_CONFIG not specified and pg_config not found in PATH. Run cmake with -DPG_CONFIG=path/to/pg_config.exe")
  endif()
endif()

# Query PostgreSQL locations using pg_config
execute_process(COMMAND "${PG_CONFIG}" --includedir
  OUTPUT_VARIABLE PG_INCLUDEDIR_RAW OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND "${PG_CONFIG}" --includedir-server
  OUTPUT_VARIABLE PG_INCLUDEDIR_SERVER_RAW OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND "${PG_CONFIG}" --libdir
  OUTPUT_VARIABLE PG_LIBDIR_RAW OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND "${PG_CONFIG}" --pkglibdir
  OUTPUT_VARIABLE PG_PKGLIBDIR_RAW OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND "${PG_CONFIG}" --pkglibdir
  OUTPUT_VARIABLE PG_PKG_LIBDIR OUTPUT_STRIP_TRAILING_WHITESPACE)

message(STATUS "PostgreSQL include dir: ${PG_INCLUDEDIR_RAW}")
message(STATUS "PostgreSQL include-server dir: ${PG_INCLUDEDIR_SERVER_RAW}")
message(STATUS "PostgreSQL lib dir: ${PG_LIBDIR_RAW}")
message(STATUS "PostgreSQL pkglibdir (install target): ${PG_PKGLIBDIR_RAW}")

set(PG_INCLUDEDIR "${PG_INCLUDEDIR_RAW}" CACHE PATH "Postgres include dir")
set(PG_INCLUDEDIR_SERVER "${PG_INCLUDEDIR_SERVER_RAW}" CACHE PATH "Postgres server include dir")
set(PG_LIBDIR "${PG_LIBDIR_RAW}" CACHE PATH "Postgres lib dir")
set(PG_PKGLIBDIR "${PG_PKGLIBDIR_RAW}" CACHE PATH "Postgres pkglibdir (for install target)")

# Add scws and zhparser subprojects
add_subdirectory(scws)
add_subdirectory(zhparser)

# Convenience target: copy built dlls into pg lib dir (requires admin or appropriate permission)
add_custom_target(install-to-pg
  COMMAND ${CMAKE_COMMAND} -E echo "Installing DLLs and extension files to PostgreSQL directories..."
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:zhparser>
    "${PG_PKGLIBDIR}/$<TARGET_FILE_NAME:zhparser>"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:scws>
    "${PG_PKGLIBDIR}/$<TARGET_FILE_NAME:scws>"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/zhparser/zhparser--2.3.sql"
    "${CMAKE_BINARY_DIR}/zhparser--2.3.sql"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/zhparser/zhparser--2.3.sql"
    "${PG_INCLUDEDIR_SERVER}/../share/extension/zhparser--2.3.sql"
  COMMENT "Copied zhparser.dll and scws.dll to PostgreSQL pkglibdir (you may need admin privileges)."
)