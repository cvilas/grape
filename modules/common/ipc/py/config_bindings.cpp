//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <nanobind/stl/string.h>

#include "bindings.h"
#include "grape/ipc/config.h"

namespace grape::ipc::py {

void bindConfig(nanobind::module_& module) {
  nanobind::enum_<Config::Scope>(module, "Scope")
      .value("Host", Config::Scope::Host)
      .value("Network", Config::Scope::Network)
      .export_values();

  nanobind::class_<Config>(module, "Config")
      .def(nanobind::init<>())
      .def_rw("name", &Config::name)
      .def_rw("scope", &Config::scope)
      .def("__repr__", [](const Config& config) -> std::string {
        return "Config(name='" + config.name +
               "', scope=" + (config.scope == Config::Scope::Host ? "Host" : "Network") + ")";
      });
}
}  // namespace grape::ipc::py
