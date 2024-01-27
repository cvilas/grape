//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <chrono>
#include <source_location>
#include <string>

#include "grape/log/severity.h"

namespace grape::log {

/// A single log record
struct [[nodiscard]] Record {
  std::chrono::time_point<std::chrono::system_clock> timestamp;
  std::source_location location;
  std::string logger_name;
  std::string message;
  Severity severity;
};
}  // namespace grape::log