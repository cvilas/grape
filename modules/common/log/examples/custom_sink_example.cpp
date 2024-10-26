//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <fstream>

#include "grape/exception.h"
#include "grape/log/logger.h"
#include "grape/log/macros.h"

//=================================================================================================
// Demonstrates how to redirect logs to a custom output stream in a custom format
auto main(int argc, const char* argv[]) -> int {
  (void)argc;
  (void)argv;
  try {
    auto log_file = std::ofstream("logs.txt");
    grape::log::Config config;
    config.sink = [&log_file](const grape::log::Record& r) {
      log_file << std::format("[{}] {}\n", r.timestamp, r.message.cStr());
    };

    auto logger = grape::log::Logger(std::move(config));
    GRAPE_LOG(logger, grape::log::Severity::Info, "{}", "Message to custom output stream");
    return EXIT_SUCCESS;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}
