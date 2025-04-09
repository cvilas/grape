//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <pybind11/stl.h>

#include "bindings.h"
#include "grape/ipc/config.h"

namespace grape::ipc::py {

void bindConfig(pybind11::module_& module) {
  pybind11::enum_<Config::Scope>(module, "Scope")
      .value("Host", Config::Scope::Host)
      .value("Network", Config::Scope::Network)
      .export_values();

  pybind11::class_<Config>(module, "Config")
      .def(pybind11::init<>())
      .def_readwrite("name", &Config::name)
      .def_readwrite("scope", &Config::scope)
      .def("__repr__", [](const Config& config) {
        return "Config(name='" + config.name +
               "', scope=" + (config.scope == Config::Scope::Host ? "Host" : "Network") + ")";
      });
}
}  // namespace grape::ipc::py
