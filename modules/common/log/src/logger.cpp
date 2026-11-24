//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "grape/log/logger.h"

#include <cassert>
#include <condition_variable>
#include <cstdio>  // for fputs, stderr
#include <mutex>
#include <string>
#include <thread>

#include "grape/exception.h"
#include "grape/log/severity.h"
#include "grape/log/sinks/sink.h"

namespace grape::log {

struct Logger::Backend {
  std::uint32_t missed_logs{ 0 };
  std::mutex wait_mutex;
  std::condition_variable_any wait_condition;
  std::jthread sink_thread;
};

//-------------------------------------------------------------------------------------------------
Logger::Logger(Config&& config)                                                       //
  : config_(std::move(config))                                                        //
  , queue_({ .frame_length = sizeof(Record), .num_frames = config_.queue_capacity })  //
  , backend_(std::make_unique<Backend>()) {
  backend_->sink_thread = std::jthread([this](const std::stop_token& st) -> auto { sinkLoop(st); });
}

//-------------------------------------------------------------------------------------------------
Logger::~Logger() {
  backend_->sink_thread.request_stop();
  backend_->sink_thread.join();
}

//-------------------------------------------------------------------------------------------------
void Logger::sinkLoop(const std::stop_token& st) noexcept {
  for (;;) {
    {
      auto lock = std::unique_lock{ backend_->wait_mutex };
      if (backend_->wait_condition.wait_for(lock, st, config_.flush_period,
                                            [&st] { return st.stop_requested(); })) {
        break;
      }
    }
    flush();
  }
  flush();
}

//-------------------------------------------------------------------------------------------------
void Logger::flush() noexcept {
  try {
    const auto record_reader = [this](std::span<const std::byte> frame) -> void {
      assert(sizeof(Record) == frame.size_bytes());
      if (config_.sink != nullptr) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const auto* const record = reinterpret_cast<const Record*>(frame.data());
        config_.sink->write(*record);
      }
    };

    // flush all log records
    while (queue_.visitToRead(record_reader)) {
    }

    // make a note of number of logs missed
    const auto missed_logs = missed_logs_.load(std::memory_order_relaxed);
    if (backend_->missed_logs != missed_logs) {
      const auto delta_missed_logs = missed_logs - backend_->missed_logs;
      backend_->missed_logs = missed_logs;
      if (config_.sink != nullptr) {
        config_.sink->write({
            .timestamp{ WallClock::now() },                      //
            .location{ std::source_location::current() },        //
            .logger_name{ config_.logger_name },                 //
            .message{ "{} records missed", delta_missed_logs },  //
            .severity = Severity::Warn,
        });
      }
    }
  } catch (...) {
    (void)std::fputs("Ignored exception in Logger::flush()", stderr);
    grape::Exception::print();
  }
}
}  // namespace grape::log
