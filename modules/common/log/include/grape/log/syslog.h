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

//-------------------------------------------------------------------------------------------------
// Specialised system logging interfaces.
// @param fmt message format string
// @param args Message args to be formatted
//-------------------------------------------------------------------------------------------------

template <typename... Args>
struct Critical {
  explicit Critical(std::format_string<Args...> fmt, Args&&... args,
                    const std::source_location& loc = std::source_location::current()) {
    instance().log(log::Severity::Critical, loc, fmt, std::forward<Args>(args)...);
  }
};

template <typename... Args>
struct Error {
  explicit Error(std::format_string<Args...> fmt, Args&&... args,
                 const std::source_location& loc = std::source_location::current()) {
    instance().log(log::Severity::Error, loc, fmt, std::forward<Args>(args)...);
  }
};

template <typename... Args>
struct Warn {
  explicit Warn(std::format_string<Args...> fmt, Args&&... args,
                const std::source_location& loc = std::source_location::current()) {
    instance().log(log::Severity::Warn, loc, fmt, std::forward<Args>(args)...);
  }
};

template <typename... Args>
struct Note {
  explicit Note(std::format_string<Args...> fmt, Args&&... args,
                const std::source_location& loc = std::source_location::current()) {
    instance().log(log::Severity::Note, loc, fmt, std::forward<Args>(args)...);
  }
};

template <typename... Args>
struct Info {
  explicit Info(std::format_string<Args...> fmt, Args&&... args,
                const std::source_location& loc = std::source_location::current()) {
    instance().log(log::Severity::Info, loc, fmt, std::forward<Args>(args)...);
  }
};

template <typename... Args>
struct Debug {
  explicit Debug(std::format_string<Args...> fmt, Args&&... args,
                 const std::source_location& loc = std::source_location::current()) {
    instance().log(log::Severity::Info, loc, fmt, std::forward<Args>(args)...);
  }
};

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
