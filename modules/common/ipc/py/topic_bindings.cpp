//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "bindings.h"
#include "grape/ipc/topic.h"

namespace grape::ipc::py {

void bindTopic(pybind11::module_& module) {
  pybind11::class_<Topic>(module, "Topic")
      .def(pybind11::init<>())
      .def_readwrite("name", &Topic::name);
}
}  // namespace grape::ipc::py
