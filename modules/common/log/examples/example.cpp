//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "grape/exception.h"
#include "grape/log/logger.h"
#include "grape/log/macros.h"
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
    GRAPE_LOG(logger, grape::log::Severity::Error, "A log message");
    GRAPE_LOG(logger, grape::log::Severity::Note, "message='{}'", "A formatted log message");
    return EXIT_SUCCESS;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}
