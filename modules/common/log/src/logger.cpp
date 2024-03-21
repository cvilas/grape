//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "grape/log/logger.h"

#include <thread>

namespace grape::log {

struct Logger::Backend {
  std::uint32_t missed_logs{ 0 };
  std::atomic_flag exit_flag{ false };
  std::thread sink_thread;
};

//-------------------------------------------------------------------------------------------------
Logger::Logger(Config&& config)     //
  : config_(std::move(config))      //
  , queue_(config_.queue_capacity)  //
  , backend_(std::make_unique<Backend>()) {
  backend_->sink_thread = std::thread([this]() { sinkLoop(); });
}

//-------------------------------------------------------------------------------------------------
Logger::~Logger() {
  backend_->exit_flag.test_and_set();
  backend_->sink_thread.join();
}

//-------------------------------------------------------------------------------------------------
void Logger::log(Record&& record) {
  if (canLog(record.severity)) {
    if (not queue_.tryPush(std::move(record))) {
      missed_logs_.fetch_add(1, std::memory_order_acquire);
    }
  }
}

//-------------------------------------------------------------------------------------------------
void Logger::sinkLoop() noexcept {
  while (not backend_->exit_flag.test()) {
    std::this_thread::sleep_for(config_.flush_period);
    flush();
  }
  flush();
}

//-------------------------------------------------------------------------------------------------
void Logger::flush() noexcept {
  try {
    // flush all log records
    for (auto record = queue_.tryPop(); record.has_value(); record = queue_.tryPop()) {
      if (config_.sink != nullptr) {
        config_.sink(record.value());  // NOLINT(bugprone-unchecked-optional-access)
      }
    }

    // make a note of number of logs missed
    const auto missed_logs = missed_logs_.load(std::memory_order_relaxed);
    if (backend_->missed_logs != missed_logs) {
      const auto delta_missed_logs = missed_logs - backend_->missed_logs;
      backend_->missed_logs = missed_logs;
      if (config_.sink != nullptr) {
        config_.sink({ .timestamp = std::chrono::system_clock::now(),                           //
                       .location = std::source_location::current(),                             //
                       .logger_name = config_.logger_name.c_str(),                              //
                       .message = std::format("{} records missed", delta_missed_logs).c_str(),  //
                       .severity = Severity::Warn });
      }
    }
  } catch (...) {
    std::ignore = std::fputs("Ignored exception in Logger::flush()\n", stderr);
  }
}
}  // namespace grape::log
