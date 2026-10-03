//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <utility>

#include "catch2/catch_test_macros.hpp"
#include "grape/exception.h"
#include "grape/log/config.h"
#include "grape/log/syslog.h"

namespace {

//-------------------------------------------------------------------------------------------------
TEST_CASE("Explicit initialisation preserves custom configuration", "[syslog]") {
  auto config = grape::log::Config{};
  config.threshold = grape::log::Severity::Warn;
  grape::syslog::init(std::move(config));

  const auto& logger = grape::syslog::instance();
  CHECK(logger.canLog(grape::log::Severity::Warn));
  CHECK_FALSE(logger.canLog(grape::log::Severity::Debug));
  CHECK(&logger == &grape::syslog::instance());
  CHECK_THROWS_AS(grape::syslog::init(grape::log::Config{}), grape::Exception);
  CHECK_FALSE(logger.canLog(grape::log::Severity::Debug));
}

}  // namespace
