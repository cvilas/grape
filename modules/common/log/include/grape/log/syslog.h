//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/log/logger.h"

namespace grape::syslog {

/// (Optional) Initialise the system logger.
/// @note Should be called only once and before calling any logging functions
/// @note If not called, a default logger is initialised on first call to any logging function
/// @param config logger configuration
void init(log::Config&& config);

/// @return system logger instance
auto instance() -> log::Logger&;

//-------------------------------------------------------------------------------------------------
// Specialised system logging interfaces.
// @param fmt message format string
// @param args Message args to be formatted
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFINE_LOG_STRUCT(NAME, SEVERITY)                                                          \
  template <typename... Args>                                                                      \
  struct NAME {                                                                                    \
    explicit NAME(std::format_string<Args...> fmt, Args&&... args,                                 \
                  const std::source_location& loc = std::source_location::current()) {             \
      instance().log(SEVERITY, loc, fmt, std::forward<Args>(args)...);                             \
    }                                                                                              \
  };
DEFINE_LOG_STRUCT(Critical, log::Severity::Critical)
DEFINE_LOG_STRUCT(Error, log::Severity::Error)
DEFINE_LOG_STRUCT(Warn, log::Severity::Warn)
DEFINE_LOG_STRUCT(Note, log::Severity::Note)
DEFINE_LOG_STRUCT(Info, log::Severity::Info)
DEFINE_LOG_STRUCT(Debug, log::Severity::Debug)
#undef DEFINE_LOG_STRUCT

//-------------------------------------------------------------------------------------------------
// Recommended user API for system logging
//-------------------------------------------------------------------------------------------------

template <typename... Args>
Critical(std::format_string<Args...> fmt, Args&&... args) -> Critical<Args...>;
template <typename... Args>
Error(std::format_string<Args...> fmt, Args&&... args) -> Error<Args...>;
template <typename... Args>
Warn(std::format_string<Args...> fmt, Args&&... args) -> Warn<Args...>;
template <typename... Args>
Note(std::format_string<Args...> fmt, Args&&... args) -> Note<Args...>;
template <typename... Args>
Info(std::format_string<Args...> fmt, Args&&... args) -> Info<Args...>;
template <typename... Args>
Debug(std::format_string<Args...> fmt, Args&&... args) -> Debug<Args...>;

}  // namespace grape::syslog
