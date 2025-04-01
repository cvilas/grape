//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <thread>

#include "catch2/catch_test_macros.hpp"
#include "grape/log/logger.h"

namespace {

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

//-------------------------------------------------------------------------------------------------
TEST_CASE("Basic logging api works", "[log]") {
  auto config = grape::log::Config();
  config.threshold = grape::log::Severity::Debug;

  auto logger = grape::log::Logger(std::move(config));

  /// Explicitly specify variadic template arguments types list
  grape::log::Log<int, float>(logger, grape::log::Severity::Info, "{} {}", 5, 3.14F,
                              std::source_location::current());

  /// Use the deduction guide for arguments with defaulted source location
  grape::log::Log(logger, grape::log::Severity::Info, "{} {}", 5, 3.14);
}

// A log sink for testing
class TestLogSink : public grape::log::Sink {
public:
  void write(const grape::log::Record& record) override {
    num_logs_.fetch_add(1, std::memory_order_relaxed);
    stream_.append(std::format("{}", record.message.cStr()));
  }

  auto stream() const -> const std::string& {
    return stream_;
  }
  auto numLogs() const -> std::size_t {
    return num_logs_.load();
  }

private:
  std::atomic_size_t num_logs_{ 0 };
  std::string stream_;
};

//-------------------------------------------------------------------------------------------------
TEST_CASE("Custom sink and threshold settings are respected", "[log]") {
  static constexpr auto QUEUE_CAPACITY = 10U;
  auto sink = std::make_shared<TestLogSink>();
  auto config = grape::log::Config();
  config.sink = sink;
  config.queue_capacity = QUEUE_CAPACITY;
  config.threshold = grape::log::Severity::Note;

  auto logger = std::make_unique<grape::log::Logger>(std::move(config));

  const auto* const log_str = "This should appear in logs";
  grape::log::Log(*logger, grape::log::Severity::Error, "{}", log_str);  //!< above threshold
  grape::log::Log(*logger, grape::log::Severity::Debug,
                  "This should not appear in logs");  //!< below threshold
  logger.reset();  //!< destroying the logger forces queue to flush
  REQUIRE(sink->stream() == log_str);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Queue capacity and flush period are respected", "[log]") {
  using namespace std::chrono_literals;
  static constexpr auto FLUSH_PERIOD = 1s;
  static constexpr auto FLUSH_WAIT_PERIOD = FLUSH_PERIOD + 500ms;
  static constexpr auto QUEUE_CAPACITY = 5U;
  auto sink = std::make_shared<TestLogSink>();
  auto config = grape::log::Config();
  config.sink = sink;
  config.queue_capacity = QUEUE_CAPACITY;
  config.flush_period = FLUSH_PERIOD;
  config.threshold = grape::log::Severity::Debug;

  auto logger = grape::log::Logger(std::move(config));

  // push messages beyond queue capacity before the logs get flushed
  static constexpr auto NUM_MESSAGES = QUEUE_CAPACITY * 3;
  for (std::size_t i = 0; i < NUM_MESSAGES; ++i) {
    grape::log::Log(logger, grape::log::Severity::Debug, "Message no. {}", i);
  }
  REQUIRE(sink->numLogs() == 0);                   //!< not flushed yet
  std::this_thread::sleep_for(FLUSH_WAIT_PERIOD);  //!< wait for flush
  REQUIRE(sink->numLogs() == QUEUE_CAPACITY + 1);  //!< includes record indicating missed count
  REQUIRE(logger.missedLogs() == NUM_MESSAGES - QUEUE_CAPACITY);  //!< check overflow
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace
