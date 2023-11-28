//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
// MIT License
//=================================================================================================

#include <print>

#include "grape/exception.h"

// Demonstrates creating a custom exception
class CustomException : public grape::Exception {
public:
  CustomException(const std::string& msg, std::source_location loc) : Exception(msg, loc) {
  }
};

//=================================================================================================
/// Demonstrates how exceptions should be thrown and caught
auto main() -> int {
  try {
    grape::panic<CustomException>("An exception occurred");
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
  }
  return EXIT_SUCCESS;
}
