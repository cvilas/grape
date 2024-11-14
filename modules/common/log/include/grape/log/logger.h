//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <source_location>

#include "grape/log/config.h"
#include "grape/log/severity.h"
#include "grape/realtime/fifo_buffer.h"

namespace grape::log {

//=================================================================================================
/// @brief A buffered lock-free logger suitable for realtime applications
class Logger {
public:
  /// Create logger
  /// @param config Configuration parameters
  explicit Logger(Config&& config);

  /// @return true if messages at specified severity are logged
  [[nodiscard]] constexpr auto canLog(Severity severity) const noexcept -> bool {
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
    log({ .timestamp{ std::chrono::system_clock::now() },  //
          .location{ location },                           //
          .logger_name{ config_.logger_name.c_str() },     //
          .message{ fmt, std::forward<Args>(args)... },    //
          .severity{ severity } });
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
  void log(const Record& record);
  void sinkLoop() noexcept;
  void flush() noexcept;

  static_assert(std::atomic_uint32_t::is_always_lock_free);
  Config config_{};
  std::atomic_uint32_t missed_logs_{ 0 };
  realtime::FIFOBuffer queue_;
  struct Backend;
  std::unique_ptr<Backend> backend_{ nullptr };
};

//=================================================================================================
// Generic logging interface (recommended user interface for logging)
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

/// CTAD deduction guide for the above template when using defaulted source location
/// (See https://www.cppstories.com/2021/non-terminal-variadic-args/)
template <typename... Args>
Log(Logger& l, Severity s, std::format_string<Args...> fmt, Args&&... args) -> Log<Args...>;

}  // namespace grape::log
