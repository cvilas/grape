# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

find_package(Python REQUIRED COMPONENTS Interpreter)

add_dependencies(tests grape_ipc_py) # Ensure bindings get built on tests target
add_test(
  NAME ${CMAKE_PROJECT_NAME}_${MODULE_NAME}_py_test
  COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/py_test.py ${PY_WHEELS_DIR}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set_tests_properties(${CMAKE_PROJECT_NAME}_${MODULE_NAME}_py_test PROPERTIES DEPENDS grape_ipc_py)
set_tests_properties(${CMAKE_PROJECT_NAME}_${MODULE_NAME}_py_test PROPERTIES TIMEOUT 300)
