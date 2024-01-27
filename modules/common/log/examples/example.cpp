//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "grape/log/logger.h"

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  (void)argc;
  (void)argv;
  try {
    auto config = grape::log::Config();
    config.logger_name = "logger example";
    config.threshold = grape::log::Severity::Debug;
    auto logger = grape::log::Logger(std::move(config));
    logger.log(grape::log::Severity::Error, "A log message");
    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}
