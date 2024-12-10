//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <print>

#include <unistd.h>

#include "grape/log/default_formatter.h"

namespace grape::log {

/// Default log sink implementation. Just writes to standard error output
inline void defaultSink(const Record& rec) {
  if (::isatty(STDERR_FILENO) != 0) {  // color format the logs if going to terminal
    const auto color = [](Severity sev) -> std::string_view {
      switch (sev) {
          // clang-format off
        case Severity::Critical: return "\033[37;41m";  // white on red
        case Severity::Error: return "\033[31m";        // red
        case Severity::Warn: return "\033[33m";         // yellow
        case Severity::Note: [[fallthrough]];
        case Severity::Info: [[fallthrough]];
        case Severity::Debug: return "";
          // clang-format on
      }
    }(rec.severity);
    static constexpr auto RESET_COLOR = "\033[0m";
    std::println(stderr, "{}{}{}", color, defaultFormatter(rec), RESET_COLOR);
  } else {
    std::println(stderr, "{}", defaultFormatter(rec));
  }
}

}  // namespace grape::log
