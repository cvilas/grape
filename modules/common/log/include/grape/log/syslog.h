//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/log/logger.h"

namespace grape::syslog {

/// Initialise the system logger
/// @param config logger configuration
void init(log::Config&& config);

/// @return system logger instance
auto instance() -> log::Logger&;

/// General system logging interface.
template <typename... Args>
struct Log {
  Log(log::Severity sev, std::format_string<Args...> fmt, Args&&... args,
      const std::source_location& loc = std::source_location::current()) {
    instance().log(sev, loc, fmt, std::forward<Args>(args)...);
  }
};

/// Recommended user API to log a message using the system logger
/// @param sev Severity level
/// @param fmt message format string
/// @param args Message args to be formatted
template <typename... Args>
Log(log::Severity sev, std::format_string<Args...> fmt, Args&&... args) -> Log<Args...>;

}  // namespace grape::syslog
