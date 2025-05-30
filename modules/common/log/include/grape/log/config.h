//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/log/formatters/default_formatter.h"
#include "grape/log/sinks/console_sink.h"
#include "grape/utils/file_system.h"

namespace grape::log {

/// @brief  Logger configuration
struct Config {
  static constexpr auto DEFAULT_QUEUE_CAPACITY = 1000U;
  static constexpr auto DEFAULT_FLUSH_PERIOD = std::chrono::microseconds(1000);

  /// Threshold severity at which messages are logged. Eg: if set to 'Warn', only 'Warn', 'Error'
  /// and 'Critical' messages are logged
  Severity threshold{ Severity::Debug };

  /// The log receiver function
  std::shared_ptr<Sink> sink{ std::make_shared<ConsoleSink<DefaultFormatter>>() };

  /// The maximum number of records the internal buffer can hold without flushing the queue.
  /// @note To avoid overflow resulting in missed logs, set this to (>= max_logs_per_second *
  /// flush_period)
  std::size_t queue_capacity{ DEFAULT_QUEUE_CAPACITY };

  /// Interval between flushing logs into sink.
  /// @note To avoid overflowing the queue, set this to (<= queue_capacity/max_logs_per_second)
  std::chrono::microseconds flush_period{ DEFAULT_FLUSH_PERIOD };

  /// Identifying name for the logger
  std::string logger_name = utils::getProgramName();
};

}  // namespace grape::log
