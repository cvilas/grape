//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <chrono>
#include <cstdint>
#include <format>
#include <functional>
#include <iostream>
#include <source_location>

#include "grape/utils/utils.h"

namespace grape::log {

/// Logging severity levels
enum class [[nodiscard]] Severity : std::uint8_t {
  Critical,  //!< For non-recoverable errors. System may be unusable.
  Error,     //!< For conditions outside normal operating envelope.
  Warn,      //!< For conditions within but approaching limits of operating envelope.
  Note,      //!< For significant events that may or may not be unusual.
  Info,      //!< For general informational messages.
  Debug      //!< For debugging information.
};

/// @return String representation of Severity
constexpr auto toString(Severity s) -> std::string_view;

/// A single log record
struct [[nodiscard]] Record {
  std::chrono::time_point<std::chrono::system_clock> timestamp;
  std::source_location location;
  std::string message;
  Severity severity;
};

/// Signature for custom log formatters.
using Formatter = std::function<std::string(const Record& r)>;

/// Default log formatter implementation
auto defaultFormatter(const Record& r) -> std::string {
  return std::format("[{}] [{}] [{}:{}] {}\n", r.timestamp, toString(r.severity),
                     std::string(utils::truncate(r.location.file_name(), "modules")),
                     r.location.line(), r.message);
}

//=================================================================================================
/// A log writer
class Logger {
public:
  /// @brief Redefine logging stream buffer from the default stderr
  /// @param buf stream buffer object to redirect logs to
  /// @return pointer to the stream buffer previously associated with the stream
  static constexpr auto setStreamBuffer(std::streambuf* buf) -> std::streambuf* {
    return std::clog.rdbuf(buf);
  }

  /// Set a user-defined custom log formatter
  constexpr void setFormatter(Formatter&& f) {
    formatter_ = std::move(f);
  }

  /// Set threshold severity at which messages are logged
  /// Eg: if set to 'Warn', only 'Warn', 'Error' and 'Critical' messages are logged
  constexpr void setThreshold(Severity s) {
    threshold_ = s;
  }

  /// @return threshold severity level
  [[nodiscard]] constexpr auto getThreshold() const -> Severity {
    return threshold_;
  }

  /// @return true if messages at specified severity are logged
  [[nodiscard]] constexpr auto canLog(Severity severity) const -> bool {
    return (severity <= threshold_);
  }

  /// @brief Log a message
  /// @param s Severity level
  /// @param msg message string
  /// @param tp timestamp (autogenerated by default)
  /// @param loc location in source (autogenerated by default)
  constexpr void log(Severity s, const std::string& msg,
                     const std::chrono::time_point<std::chrono::system_clock>& tp =
                         std::chrono::system_clock::now(),
                     const std::source_location& loc = std::source_location::current()) const {
    log(Record{ .timestamp = tp, .location = loc, .message = msg, .severity = s });
  }

private:
  constexpr void log(const Record& record) const {
    if (canLog(record.severity)) {
      std::clog << formatter_(record);
    }
  }

  Severity threshold_{ Severity::Debug };
  Formatter formatter_{ defaultFormatter };
};

//-------------------------------------------------------------------------------------------------
inline constexpr auto toString(Severity s) -> std::string_view {
  /// @todo(vilas): replace hardcoded strings with magic_enum
  switch (s) {
    case Severity::Critical:
      return "Critical";
    case Severity::Error:
      return "Error";
    case Severity::Warn:
      return "Warn";
    case Severity::Note:
      return "Note";
    case Severity::Info:
      return "Info";
    case Severity::Debug:
      return "Debug";
  }
  return "";
}

}  // namespace grape::log