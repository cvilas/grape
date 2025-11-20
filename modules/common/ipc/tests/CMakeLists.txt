# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

# C++ tests
define_module_test(NAME host_raw_pub_sub_test SOURCES host_raw_pub_sub_test.cpp)
define_module_test(NAME typed_pub_sub_test SOURCES typed_pub_sub_test.cpp)
define_module_test(NAME network_raw_pub_sub_test SOURCES network_raw_pub_sub_test.cpp)
define_module_test(NAME session_test SOURCES session_test.cpp)

# Python tests
find_package(Python REQUIRED COMPONENTS Interpreter)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run_py_test.sh.in
               ${CMAKE_CURRENT_BINARY_DIR}/run_py_test.sh @ONLY)

add_dependencies(tests grape_ipc_py) # Ensure bindings get built on tests target
add_test(NAME ${CMAKE_PROJECT_NAME}_${MODULE_NAME}_py_test
         COMMAND ${CMAKE_CURRENT_BINARY_DIR}/run_py_test.sh)
set_tests_properties(
  ${CMAKE_PROJECT_NAME}_${MODULE_NAME}_py_test
  PROPERTIES DEPENDS grape_ipc_py ${CMAKE_CURRENT_BINARY_DIR}/host_raw_pub_sub_test.py
             ${CMAKE_CURRENT_BINARY_DIR}/run_py_test.sh)
