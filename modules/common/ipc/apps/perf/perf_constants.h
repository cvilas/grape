//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/ipc/topic.h"

namespace grape::ipc::ex::perf {
static auto topic() -> const Topic& {
  static auto topic = grape::ipc::Topic{
    .name = "grape/ipc/example/perf",
    .type_name = "bytes",
  };
  return topic;
}
}  // namespace grape::ipc::ex::perf
