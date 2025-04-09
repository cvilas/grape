//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "bindings.h"
#include "grape/ipc/match.h"

namespace grape::ipc::py {

void bindMatch(pybind11::module_& module) {
  pybind11::enum_<Match::Status>(module, "MatchStatus")
      .value("Undefined", Match::Status::Undefined)
      .value("Unmatched", Match::Status::Unmatched)
      .value("Matched", Match::Status::Matched)
      .export_values();

  pybind11::class_<Match>(module, "Match")
      .def(pybind11::init<>())
      .def_readwrite("status", &Match::status);
}
}  // namespace grape::ipc::py
