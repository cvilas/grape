//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <streambuf>

#include "grape/log/logger.h"

/// A custom buffer class that clones data to multiple data buffers.
class TeeStreamBuffer : public std::streambuf {
public:
  TeeStreamBuffer(std::streambuf* sb1, std::streambuf* sb2) : sb1_(sb1), sb2_(sb2) {
  }

  auto overflow(int_type c) -> int_type final {
    if (c != traits_type::eof()) {
      const auto ch = traits_type::to_char_type(c);
      if (sb1_->sputc(ch) == traits_type::eof() || sb2_->sputc(ch) == traits_type::eof()) {
        return traits_type::eof();
      }
    }
    return c;
  }

  auto sync() -> int final {
    if (sb1_->pubsync() == -1 || sb2_->pubsync() == -1) {
      return -1;
    }
    return 0;
  }

private:
  std::streambuf* sb1_;  // Stream buffer for the file
  std::streambuf* sb2_;  // Stream buffer for the standard output
};

//=================================================================================================
// Demonstrates redirecting log output to multiple sinks at the same time
auto main() -> int {
  // Create a custom tee buffer that redirects to a file and std::cout
  std::ofstream log_file("logs.txt");
  TeeStreamBuffer tee_buffer(log_file.rdbuf(), std::cout.rdbuf());

  // Create a new stream using the custom tee buffer
  std::ostream tee_stream(&tee_buffer);

  grape::log::Logger logger;

  // Redirect log to the custom stream
  auto* original_buffer = grape::log::Logger::setStreamBuffer(tee_stream.rdbuf());

  // Now, any output sent to logger will go to both the log file and standard output
  logger.log(grape::log::Severity::Info, "This message goes to log file and standard output");

  // Restore the original buffer before exiting
  grape::log::Logger::setStreamBuffer(original_buffer);

  return EXIT_SUCCESS;
}
