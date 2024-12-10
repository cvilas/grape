//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include "grape/utils/enums.h"

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
constexpr auto toString(Severity sev) -> std::string_view {
  return enums::name(sev);
}
}  // namespace grape::log
