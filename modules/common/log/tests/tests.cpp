//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <thread>

#include "catch2/catch_test_macros.hpp"
#include "grape/log/logger.h"

namespace {

// NOLINTBEGIN(cert-err58-cpp)

//-------------------------------------------------------------------------------------------------
TEST_CASE("Custom sink and threshold settings are respected", "[log]") {
  std::string stream;
  static constexpr auto QUEUE_CAPACITY = 10u;

  auto config = grape::log::Config();
  config.threshold = grape::log::Severity::Note;
  config.sink = [&stream](const grape::log::Record& r) {
    stream.append(std::format("{}", r.message.cStr()));
  };
  config.queue_capacity = QUEUE_CAPACITY;

  auto logger = std::make_unique<grape::log::Logger>(std::move(config));

  const std::string log_str = "This should appear in logs";
  logger->log(grape::log::Severity::Error, log_str);                           //!< above threshold
  logger->log(grape::log::Severity::Debug, "This should not appear in logs");  //!< below threshold
  logger.reset();  //!< destroying the logger forces queue to flush
  REQUIRE(stream == log_str);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Queue capacity and flush period are respected", "[log]") {
  using namespace std::chrono_literals;
  static constexpr auto FLUSH_PERIOD = 1s;
  static constexpr auto FLUSH_WAIT_PERIOD = FLUSH_PERIOD + 500ms;
  static constexpr auto QUEUE_CAPACITY = 5u;
  std::atomic_size_t num_logs{ 0 };
  auto config = grape::log::Config();
  config.threshold = grape::log::Severity::Debug;
  config.sink = [&num_logs](const grape::log::Record&) { num_logs++; };
  config.queue_capacity = QUEUE_CAPACITY;
  config.flush_period = FLUSH_PERIOD;

  auto logger = grape::log::Logger(std::move(config));

  // push messages beyond queue capacity before the logs get flushed
  static constexpr auto NUM_MESSAGES = QUEUE_CAPACITY * 3;
  for (std::size_t i = 0; i < NUM_MESSAGES; ++i) {
    logger.log(grape::log::Severity::Debug, std::format("Message no. {}", i));
  }
  REQUIRE(num_logs == 0);                          //!< not flushed yet
  std::this_thread::sleep_for(FLUSH_WAIT_PERIOD);  //!< wait for flush
  REQUIRE(num_logs == QUEUE_CAPACITY + 1);         //!< includes record indicating missed count
  REQUIRE(logger.missedLogs() == NUM_MESSAGES - QUEUE_CAPACITY);  //!< check overflow
}

// NOLINTEND(cert-err58-cpp)

}  // namespace
