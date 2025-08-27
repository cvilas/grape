//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "bindings.h"
#include "grape/ipc/match.h"

namespace grape::ipc::py {

void bindMatch(pybind11::module_& module) {
  pybind11::class_<EntityId>(module, "EntityId")
      .def(pybind11::init<>())
      .def_readwrite("host", &EntityId::host)
      .def_readwrite("id", &EntityId::id);

  pybind11::enum_<Match::Status>(module, "MatchStatus")
      .value("Unmatched", Match::Status::Unmatched)
      .value("Matched", Match::Status::Matched)
      .export_values();

  pybind11::class_<Match>(module, "Match")
      .def(pybind11::init<>())
      .def_readwrite("status", &Match::status)
      .def_readwrite("remote_entity", &Match::remote_entity);
}
}  // namespace grape::ipc::py
