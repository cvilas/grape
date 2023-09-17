//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <cstdlib>

#include "grape/log/log.h"

//=================================================================================================
// Demonstrates customisation of the logging output with a custom formatter
auto main(int argc, const char* argv[]) -> int {
  (void)argc;
  (void)argv;
  grape::log::setFormatter(
      [](const grape::log::Record& r) -> std::string { return std::format("{}\n", r.message); });
  grape::log::log(grape::log::Severity::Error, "Custom formatted log message");
  return EXIT_SUCCESS;
}
