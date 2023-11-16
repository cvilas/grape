//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <cstdlib>
#include <print>
#include <sstream>

#include "grape/log/log.h"

//=================================================================================================
// Demonstrates how to redirect the default console output to a different stream (file, string, etc)
auto main(int argc, const char* argv[]) -> int {
  (void)argc;
  (void)argv;

  // sends message to default stream buffer, which is the console
  grape::log::log(grape::log::Severity::Info, "Message to default output stream");

  // save pointer to current output stream buffer and substitute it with a custom stream buffer.
  std::ostringstream custom_stream;
  auto* previous_stream = grape::log::setStreamBuffer(custom_stream.rdbuf());

  // now logger works with custom buffer. you don't see this message on the console.
  grape::log::log(grape::log::Severity::Info, "Message to custom output stream");

  // go back to old buffer
  grape::log::setStreamBuffer(previous_stream);

  // you will see this message
  grape::log::log(grape::log::Severity::Info, "Message directed *back* to default output stream");

  // print content from custom stream
  std::println("Custom stream content:\n{}", custom_stream.str());

  return EXIT_SUCCESS;
}