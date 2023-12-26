//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <cstdlib>

#include "grape/log/logger.h"

//=================================================================================================
// Demonstrates customisation of the logging output with a custom formatter
auto main(int argc, const char* argv[]) -> int {
  (void)argc;
  (void)argv;
  grape::log::Logger logger;
  logger.setFormatter(
      [](const grape::log::Record& r) -> std::string { return std::format("{}\n", r.message); });
  logger.log(grape::log::Severity::Error, "Custom formatted log message");
  return EXIT_SUCCESS;
}
