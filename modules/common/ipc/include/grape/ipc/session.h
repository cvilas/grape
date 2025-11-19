//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/ipc/config.h"

namespace grape::ipc {

/// Initialise IPC session for the process
/// @param config Process IPC configuration
void init(const Config& config);

/// @return true if session state is nominal and error-free
[[nodiscard]] auto ok() -> bool;
}  // namespace grape::ipc
