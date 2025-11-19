//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <nanobind/stl/string.h>

#include "bindings.h"
#include "grape/ipc/config.h"
#include "grape/ipc/session.h"

namespace grape::ipc::py {

void bindSession(nanobind::module_& module) {
  module.def(
      "init", [](const Config& config) -> void { init(config); },
      "Initialize IPC session for the process", nanobind::arg("config"));

  module.def("ok", &ok, "Check if session state is nominal and error-free");
}
}  // namespace grape::ipc::py
