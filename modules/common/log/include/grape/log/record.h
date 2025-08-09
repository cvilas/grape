//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <chrono>
#include <source_location>
#include <type_traits>

#include "grape/fixed_string.h"
#include "grape/log/severity.h"

namespace grape::log {

/// A single log record
struct [[nodiscard]] Record {
  static constexpr auto MAX_LOGGER_NAME_LEN = 63U;
  static constexpr auto MAX_LOG_MESSAGE_LEN = 255U;

  std::chrono::system_clock::time_point timestamp;
  std::source_location location;
  FixedString<MAX_LOGGER_NAME_LEN> logger_name;
  FixedString<MAX_LOG_MESSAGE_LEN> message;
  Severity severity{ Severity::Debug };
};

// Ensure any future changes maintains triviality for the sake of performance
// Logger class relies on this being trivially copyable
static_assert(std::is_trivially_copyable_v<Record>);
static_assert(std::is_trivially_move_constructible_v<Record>);

}  // namespace grape::log
