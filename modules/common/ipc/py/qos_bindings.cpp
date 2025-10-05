//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "bindings.h"
#include "grape/ipc/qos.h"

namespace grape::ipc::py {

void bindQoS(nanobind::module_& module) {
  nanobind::enum_<QoS>(module, "QoS")
      .value("BestEffort", QoS::BestEffort)
      .value("Reliable", QoS::Reliable)
      .export_values();
}

}  // namespace grape::ipc::py
