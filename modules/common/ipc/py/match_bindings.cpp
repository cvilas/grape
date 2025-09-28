//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "bindings.h"
#include "grape/ipc/match.h"

namespace grape::ipc::py {

void bindMatch(nanobind::module_& module) {
  nanobind::class_<EntityId>(module, "EntityId")
      .def(nanobind::init<>())
      .def_rw("host", &EntityId::host)
      .def_rw("id", &EntityId::id);

  nanobind::enum_<Match::Status>(module, "MatchStatus")
      .value("Unmatched", Match::Status::Unmatched)
      .value("Matched", Match::Status::Matched)
      .export_values();

  nanobind::class_<Match>(module, "Match")
      .def(nanobind::init<>())
      .def_rw("status", &Match::status)
      .def_rw("remote_entity", &Match::remote_entity);
}
}  // namespace grape::ipc::py
