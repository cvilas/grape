//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <nanobind/nanobind.h>

namespace grape::ipc::py {
void bindConfig(const nanobind::module_& module);
void bindSession(nanobind::module_& module);
void bindMatch(const nanobind::module_& module);
void bindQoS(const nanobind::module_& module);
void bindPublisher(const nanobind::module_& module);
void bindSubscriber(const nanobind::module_& module);
}  // namespace grape::ipc::py
