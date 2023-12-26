//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <cstdlib>
#include <print>
#include <sstream>

#include "grape/log/logger.h"

//=================================================================================================
// Demonstrates how to redirect the default console output to a different stream (file, string, etc)
auto main(int argc, const char* argv[]) -> int {
  (void)argc;
  (void)argv;
  try {
    grape::log::Logger logger;

    // sends message to default stream buffer, which is the console
    logger.log(grape::log::Severity::Info, "Message to default output stream");

    // save pointer to current output stream buffer and substitute it with a custom stream buffer.
    std::ostringstream custom_stream;
    auto* previous_stream = grape::log::Logger::setStreamBuffer(custom_stream.rdbuf());

    // now logger works with custom buffer. you don't see this message on the console.
    logger.log(grape::log::Severity::Info, "Message to custom output stream");

    // go back to old buffer
    grape::log::Logger::setStreamBuffer(previous_stream);

    // you will see this message
    logger.log(grape::log::Severity::Info, "Message directed *back* to default output stream");

    // print content from custom stream
    std::println("Custom stream content:\n{}", custom_stream.str());

    return EXIT_SUCCESS;

  } catch (...) {
    std::ignore = std::fputs("Exception occurred", stderr);
    return EXIT_FAILURE;
  }
}
