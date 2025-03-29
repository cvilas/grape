//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/log/logger.h"
#include "grape/utils/utils.h"

namespace grape::syslog {

/// @return Creates the system logger
static auto systemLogger() -> log::Logger& {
  static log::Logger logger(log::Config{ .logger_name = utils::getProgramName() });
  return logger;
}

/// System logging interface
template <typename... Args>
struct Log {
  Log(log::Severity sev, std::format_string<Args...> fmt, Args&&... args,
      const std::source_location& loc = std::source_location::current()) {
    systemLogger().log(sev, loc, fmt, std::forward<Args>(args)...);
  }
};

//=================================================================================================
// User API for system logging
//=================================================================================================

/// Log a message using the system logger
/// @param sev Severity level
/// @param fmt message format string
/// @param args Message args to be formatted
template <typename... Args>
Log(log::Severity sev, std::format_string<Args...> fmt, Args&&... args) -> Log<Args...>;

/// Set threshold severity for system logging services
/// @param sev Threshold
inline void setThreshold(log::Severity sev) {
  systemLogger().setThreshold(sev);
}
}  // namespace grape::syslog
