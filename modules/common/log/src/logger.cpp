//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "grape/log/logger.h"

#include <cstring>  // for memcpy
#include <thread>

#include "grape/exception.h"

namespace grape::log {

struct Logger::Backend {
  std::uint32_t missed_logs{ 0 };
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
void Logger::log(const Record& record) {
  if (not canLog(record.severity)) {
    return;
  }
  const auto writer = [&record](std::span<std::byte> frame) -> void {
    assert(sizeof(record) == frame.size_bytes());
    std::memcpy(frame.data(), &record, sizeof(Record));
  };
  if (not queue_.visitToWrite(writer)) {
    missed_logs_.fetch_add(1, std::memory_order_relaxed);
  }
}

//-------------------------------------------------------------------------------------------------
void Logger::sinkLoop(const std::stop_token& st) noexcept {
  while (not st.stop_requested()) {
    std::this_thread::sleep_for(config_.flush_period);
    flush();
  }
  flush();
}

//-------------------------------------------------------------------------------------------------
void Logger::flush() noexcept {
  try {
    auto record_reader = [this](std::span<const std::byte> frame) -> void {
      assert(sizeof(Record) == frame.size_bytes());
      if (config_.sink != nullptr) {
        /// @note: Sets logger name in the record here instead of in the logging hot path. This
        /// requires casting away const qualifier on the record frame. Should be safe to do since
        /// data types are fixed size and only this location reads the queue
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const auto* const const_record = reinterpret_cast<const Record*>(frame.data());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        auto* record = const_cast<Record*>(const_record);
        record->logger_name.append(config_.logger_name.c_str());
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
        config_.sink->write({ .timestamp{ std::chrono::system_clock::now() },      //
                              .location{ std::source_location::current() },        //
                              .logger_name{ config_.logger_name.c_str() },         //
                              .message{ "{} records missed", delta_missed_logs },  //
                              .severity = Severity::Warn });
      }
    }
  } catch (...) {
    (void)std::fputs("Ignored exception in Logger::flush()", stderr);
    grape::Exception::print();
  }
}
}  // namespace grape::log
