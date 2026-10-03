//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <array>
#include <barrier>
#include <cstddef>
#include <thread>
#include <vector>

#include "catch2/catch_test_macros.hpp"
#include "grape/exception.h"
#include "grape/log/config.h"
#include "grape/log/syslog.h"

namespace {

//-------------------------------------------------------------------------------------------------
TEST_CASE("Concurrent first access returns the same default logger", "[syslog]") {
  static constexpr auto NUM_THREADS = 8U;
  std::array<grape::log::Logger*, NUM_THREADS> loggers{};
  std::barrier<> start{ static_cast<std::ptrdiff_t>(NUM_THREADS) };
  {
    std::vector<std::jthread> threads;
    threads.reserve(NUM_THREADS);
    for (auto& logger : loggers) {
      threads.emplace_back([&start, &logger] {
        start.arrive_and_wait();
        logger = &grape::syslog::instance();
      });
    }
  }

  for (const auto* logger : loggers) {
    CHECK(logger == &grape::syslog::instance());
    CHECK(logger->canLog(grape::log::Severity::Debug));
  }
  CHECK_THROWS_AS(grape::syslog::init(grape::log::Config{}), grape::Exception);
}

}  // namespace
