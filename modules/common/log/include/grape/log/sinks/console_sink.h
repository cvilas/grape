//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <print>

#include <unistd.h>

#include "grape/log/sink.h"

namespace grape::log {

/// Log sink that just writes to standard error output
template <Formatter F>
struct ConsoleSink : public Sink {
  void write(const Record& record) override {
    if (::isatty(STDERR_FILENO) != 0) {  // color format the logs if going to terminal
      const auto* const color = [](Severity sev) {
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
        return "";
      }(record.severity);
      static constexpr auto RESET_COLOR = "\033[0m";
      std::println(stderr, "{}{}{}", color, F::format(record), RESET_COLOR);
    } else {
      std::println(stderr, "{}", F::format(record));
    }
  }
};
}  // namespace grape::log
