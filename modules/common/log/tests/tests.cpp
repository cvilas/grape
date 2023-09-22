//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include "catch2/catch_test_macros.hpp"
#include "grape/log/log.h"

namespace grape::log::tests {

// NOLINTBEGIN(cert-err58-cpp)

TEST_CASE("Logging tests", "[log]") {
  // set a custom formatter so we know what the log would look like
  grape::log::setFormatter(
      [](const grape::log::Record& r) -> std::string { return std::format("{}", r.message); });

  // set a logging level so we can check logs higher in severity are logged but not lower
  grape::log::setThreshold(grape::log::Severity::Note);

  // set a custom stream so we can examine it
  std::ostringstream test_ostream;
  auto* previous_stream = grape::log::setStreamBuffer(test_ostream.rdbuf());

  const std::string log_str = "This should appear in logs";
  grape::log::log(grape::log::Severity::Error, log_str);
  grape::log::log(grape::log::Severity::Debug, "This should not appear in logs");
  CHECK(test_ostream.str() == log_str);

  // reset logging stream
  grape::log::setStreamBuffer(previous_stream);
}

// NOLINTEND(cert-err58-cpp)

}  // namespace grape::log::tests
