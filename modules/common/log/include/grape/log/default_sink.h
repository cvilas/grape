//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <print>

#include "grape/log/default_formatter.h"

namespace grape::log {

/// Default log sink implementation. Just writes to standard error output
inline void defaultSink(const Record& r) {
  std::println(stderr, "{}", defaultFormatter(r));
}

}  // namespace grape::log
