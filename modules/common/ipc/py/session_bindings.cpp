//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <pybind11/functional.h>

#include "bindings.h"
#include "grape/ipc/config.h"
#include "grape/ipc/session.h"

namespace grape::ipc::py {

void bindSession(pybind11::module_& module) {
  module.def(
      "init", [](Config config) -> void { init(std::move(config)); },
      "Initialize IPC session for the process", pybind11::arg("config"));

  module.def("ok", &ok, "Check if session state is nominal and error-free");
}
}  // namespace grape::ipc::py
