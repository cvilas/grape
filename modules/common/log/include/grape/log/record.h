//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <chrono>
#include <source_location>
#include <type_traits>

#include "grape/log/severity.h"
#include "grape/realtime/fixed_string.h"

namespace grape::log {

/// A single log record
struct [[nodiscard]] Record {
  static constexpr auto MAX_LOGGER_NAME_LEN = 63U;
  static constexpr auto MAX_LOG_MESSAGE_LEN = 255U;

  std::chrono::time_point<std::chrono::system_clock> timestamp;
  std::source_location location;
  realtime::FixedString<MAX_LOGGER_NAME_LEN> logger_name;
  realtime::FixedString<MAX_LOG_MESSAGE_LEN> message;
  Severity severity{ Severity::Debug };
};

// Ensure any future changes maintains triviality for the sake of performance
static_assert(std::is_trivially_copyable_v<Record> == true);
static_assert(std::is_trivially_move_constructible_v<Record> == true);

}  // namespace grape::log