//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "bindings.h"

PYBIND11_MODULE(grape_ipc_py, module) {
  module.doc() = "Python bindings for grape::ipc";
  grape::ipc::py::bindConfig(module);
  grape::ipc::py::bindSession(module);
  grape::ipc::py::bindMatch(module);
  grape::ipc::py::bindPublisher(module);
  grape::ipc::py::bindSubscriber(module);
}
