find_package(pybind11 REQUIRED)

set(SOURCES
   py_bindings.cpp
)

add_library(grape_ipc_py MODULE ${SOURCES})
target_link_libraries(grape_ipc_py PRIVATE grape::ipc pybind11::module)
set_target_properties(grape_ipc_py PROPERTIES PREFIX "" OUTPUT_NAME "grape_ipc_py")