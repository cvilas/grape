//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/log/syslog.h"

#include <memory>
#include <mutex>

#include "grape/exception.h"

namespace {
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
std::once_flag s_init_flag;
std::unique_ptr<grape::log::Logger> s_logger{ nullptr };
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)
}  // namespace

namespace grape::syslog {

//-------------------------------------------------------------------------------------------------
void init(log::Config&& config) {
  auto succeeded = false;
  std::call_once(s_init_flag, [&succeeded, &config]() {
    s_logger = std::make_unique<log::Logger>(std::move(config));
    succeeded = true;
  });
  if (not succeeded) {
    panic<Exception>("init() must be called only once and before using logging functions");
  }
}

//-------------------------------------------------------------------------------------------------
auto instance() -> log::Logger& {
  if (s_logger != nullptr) [[likely]] {
    return *s_logger;
  }

  std::call_once(s_init_flag, []() { s_logger = std::make_unique<log::Logger>(log::Config{}); });

  return *s_logger;
}

}  // namespace grape::syslog
