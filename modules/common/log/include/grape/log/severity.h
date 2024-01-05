//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>
#include <string_view>

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
constexpr auto toString(Severity s) -> std::string_view {
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
