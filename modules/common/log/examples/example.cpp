//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "grape/exception.h"
#include "grape/log/logger.h"
#include "grape/log/severity.h"

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  (void)argc;
  (void)argv;
  try {
    auto config = grape::log::Config();
    config.logger_name = "logger example";
    config.threshold = grape::log::Severity::Debug;
    auto logger = grape::log::Logger(std::move(config));
    grape::log::Log(logger, grape::log::Severity::Error, "A log message");
    grape::log::Log(logger, grape::log::Severity::Note, "message='{}'", "A formatted log message");
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
