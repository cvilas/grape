//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include "grape/log/logger.h"

#include <thread>

namespace grape::log {

//-------------------------------------------------------------------------------------------------
Logger::Logger(Config&& config)     //
  : config_(std::move(config))      //
  , queue_(config_.queue_capacity)  //
  , sink_thread_([this]() { sinkLoop(); }) {
}

//-------------------------------------------------------------------------------------------------
Logger::~Logger() {
  exit_flag_.test_and_set();
  sink_thread_.join();
}

//-------------------------------------------------------------------------------------------------
void Logger::log(Record&& record) noexcept {
  if (canLog(record.severity)) {
    if (not queue_.tryPush(std::move(record))) {
      missed_logs_.fetch_add(1, std::memory_order_acquire);
    }
  }
}

//-------------------------------------------------------------------------------------------------
void Logger::sinkLoop() noexcept {
  while (not exit_flag_.test()) {
    std::this_thread::sleep_for(config_.flush_period);
    flush();
  }
  flush();
}

//-------------------------------------------------------------------------------------------------
void Logger::flush() noexcept {
  try {
    for (auto record = queue_.tryPop(); record.has_value(); record = queue_.tryPop()) {
      if (config_.sink != nullptr) {
        config_.sink(record.value());  // NOLINT(bugprone-unchecked-optional-access)
      }
    }
  } catch (...) {
    std::ignore = std::fputs("Ignored exception in Logger::flush()\n", stderr);
  }
}
}  // namespace grape::log
