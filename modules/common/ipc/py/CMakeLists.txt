find_package(pybind11 REQUIRED)

set(SOURCES
   py_bindings.cpp
)

# Build python module
add_library(grape_ipc_py MODULE ${SOURCES})
target_link_libraries(grape_ipc_py PRIVATE grape::ipc pybind11::module)
set_target_properties(grape_ipc_py PROPERTIES PREFIX "" OUTPUT_NAME "grape_ipc_py")

# Define the Python package directory
set(PYTHON_PACKAGE_DIR ${CMAKE_BINARY_DIR}/python_package)

# Copy the built module to the Python package directory
add_custom_command(TARGET grape_ipc_py POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PYTHON_PACKAGE_DIR}/grape_ipc
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:grape_ipc_py> ${PYTHON_PACKAGE_DIR}/grape_ipc/
)

# Add a custom target to package the Python module
add_custom_target(package_python ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/setup.py ${PYTHON_PACKAGE_DIR}/
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/grape_ipc ${PYTHON_PACKAGE_DIR}/grape_ipc
    WORKING_DIRECTORY ${PYTHON_PACKAGE_DIR}
    COMMENT "Preparing Python package in ${PYTHON_PACKAGE_DIR}"
)