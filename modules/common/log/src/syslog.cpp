//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/log/syslog.h"

#include <mutex>  // for call_once, once_flag
#include <optional>
#include <utility>

#include "grape/exception.h"
#include "grape/log/config.h"
#include "grape/log/logger.h"

namespace {
struct State {
  std::once_flag init_flag;
  std::optional<grape::log::Logger> logger;
};

auto state() -> State& {
  static State instance;
  return instance;
}
}  // namespace

namespace grape::syslog {

//-------------------------------------------------------------------------------------------------
void init(log::Config&& config) {
  auto& shared = state();
  auto succeeded = false;
  std::call_once(shared.init_flag, [&] {
    shared.logger.emplace(std::move(config));
    succeeded = true;
  });
  if (not succeeded) {
    panic("init() must be called only once and before using logging functions");
  }
}

//-------------------------------------------------------------------------------------------------
auto instance() -> log::Logger& {
  auto& shared = state();
  std::call_once(shared.init_flag, [&] { shared.logger.emplace(log::Config{}); });
  if (not shared.logger) {
    panic("Logger initialization failed");
  }
  return *shared.logger;
}

}  // namespace grape::syslog
