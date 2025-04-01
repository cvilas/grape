//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <fstream>

#include "grape/exception.h"
#include "grape/log/logger.h"

//=================================================================================================
// Custom log sink implementation
struct CustomSink : public grape::log::Sink {
  void write(const grape::log::Record& rec) override {
    std::println("[{}] {}", rec.timestamp, rec.message.cStr());
  }
};

//=================================================================================================
// Demonstrates how to redirect logs to a custom output stream in a custom format
auto main() -> int {
  try {
    auto log_file = std::ofstream("logs.txt");
    grape::log::Config config;
    config.sink = std::make_shared<CustomSink>();
    auto logger = grape::log::Logger(std::move(config));
    grape::log::Log(logger, grape::log::Severity::Info, "{}", "Message to custom output stream");
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
