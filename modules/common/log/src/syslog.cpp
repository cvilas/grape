//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/log/syslog.h"

#include <mutex>
#include <optional>

#include "grape/exception.h"

namespace {
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
std::once_flag s_init_flag;
std::optional<grape::log::Logger> s_logger{ std::nullopt };
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)
}  // namespace

namespace grape::syslog {

//-------------------------------------------------------------------------------------------------
void init(log::Config&& config) {
  auto succeeded = false;
  std::call_once(s_init_flag, [&succeeded, &config]() -> void {
    s_logger.emplace(std::move(config));
    succeeded = true;
  });
  if (not succeeded) {
    panic<Exception>("init() must be called only once and before using logging functions");
  }
}

//-------------------------------------------------------------------------------------------------
auto instance() -> log::Logger& {
  if (s_logger) [[likely]] {
    return *s_logger;
  }
  std::call_once(s_init_flag, []() -> void { s_logger.emplace(log::Config{}); });
  return *s_logger;  // NOLINT(bugprone-unchecked-optional-access)
}

}  // namespace grape::syslog
