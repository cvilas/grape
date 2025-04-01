//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/log/syslog.h"

#include "grape/exception.h"

namespace {
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::optional<grape::log::Logger> s_logger = std::nullopt;
}  // namespace

namespace grape::syslog {

//-------------------------------------------------------------------------------------------------
void init(log::Config&& config) {
  if (s_logger.has_value()) {
    panic<Exception>("Already initialised");
  }
  s_logger.emplace(std::move(config));
}

//-------------------------------------------------------------------------------------------------
auto instance() -> log::Logger& {
  if (not s_logger) {
    panic<Exception>("Not initialised");
  }
  return s_logger.value();  // NOLINT(bugprone-unchecked-optional-access)
}

}  // namespace grape::syslog
