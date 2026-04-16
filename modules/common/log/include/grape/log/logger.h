//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstring>
#include <format>
#include <source_location>
#include <stop_token>

#include "grape/fifo_buffer.h"
#include "grape/log/config.h"
#include "grape/log/record.h"
#include "grape/log/severity.h"

namespace grape::log {

//=================================================================================================
/// A buffered lock-free logger suitable for realtime applications
class Logger {
public:
  /// Create logger
  /// @param config Configuration parameters
  explicit Logger(Config&& config);

  /// @return true if messages at specified severity are logged
  [[nodiscard]] auto canLog(Severity severity) const noexcept -> bool {
    return (severity <= config_.threshold);
  }

  /// Log a message
  /// @param severity Severity level
  /// @param location location in source
  /// @param fmt message format string
  /// @param args Message args to be formatted
  template <typename... Args>
  void log(Severity severity, const std::source_location& location,
           const std::format_string<Args...> fmt, Args&&... args) {
    if (not canLog(severity)) [[unlikely]] {
      return;
    }
    const auto record = Record{
      .timestamp{ WallClock::now() },                //
      .location{ location },                         //
      .logger_name{ config_.logger_name },           //
      .message{ fmt, std::forward<Args>(args)... },  //
      .severity{ severity },                         //
    };
    const auto writer = [&record](std::span<std::byte> frame) noexcept {
      std::memcpy(frame.data(), &record, sizeof(Record));
    };
    if (not queue_.visitToWrite(writer)) [[unlikely]] {
      missed_logs_.fetch_add(1, std::memory_order_relaxed);
    }
  }

  /// @return Total number of logs that were missed due to queue overflow
  [[nodiscard]] auto missedLogs() const noexcept -> std::uint32_t {
    return missed_logs_.load(std::memory_order_relaxed);
  }

  ~Logger();
  Logger(Logger const&) = delete;
  Logger(Logger&&) = delete;
  void operator=(Logger const&) = delete;
  void operator=(Logger&&) = delete;

private:
  void sinkLoop(const std::stop_token& st) noexcept;
  void flush() noexcept;

  Config config_{};
  static_assert(std::atomic_uint32_t::is_always_lock_free);
  std::atomic_uint32_t missed_logs_{ 0 };
  FIFOBuffer queue_;
  struct Backend;
  std::unique_ptr<Backend> backend_{ nullptr };
};

//=================================================================================================
// Recommended user interface for logging
template <typename... Args>
struct Log {
  /// Log a message to any logger
  /// @param logger The specific logger to use
  /// @param severity Severity level
  /// @param fmt message format string
  /// @param args Message args to be formatted
  /// @param location location in source
  Log(Logger& logger, Severity severity, std::format_string<Args...> fmt, Args&&... args,
      const std::source_location& location = std::source_location::current()) {
    logger.log(severity, location, fmt, std::forward<Args>(args)...);
  }
};

/// CTAD deduction with defaulted source location
template <typename... Args>
Log(Logger& logger, Severity sev, std::format_string<Args...> fmt, Args&&... args) -> Log<Args...>;

}  // namespace grape::log
