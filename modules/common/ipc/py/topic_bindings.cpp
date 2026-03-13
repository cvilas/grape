//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <nanobind/stl/string.h>

#include "bindings.h"
#include "grape/ipc/topic.h"

namespace grape::ipc::py {

void bindTopic(nanobind::module_& module) {
  nanobind::class_<Topic>(module, "Topic")
      .def(nanobind::init<>())
      .def_rw("name", &Topic::name)
      .def_rw("type_name", &Topic::type_name);
}
}  // namespace grape::ipc::py
