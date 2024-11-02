//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
// MIT License
//=================================================================================================

#include <format>

#include "grape/exception.h"

namespace {
enum class Error : uint8_t { Bad, RealBad };
constexpr auto toString(const Error& er) -> std::string_view {
  switch (er) {
    case Error::Bad:
      return "Bad";
    case Error::RealBad:
      return "RealBad";
  }
};

void functionThatThrows() {
  grape::panic<grape::Exception>(std::format("Boom!! [{}]", toString(Error::RealBad)));
}

void doWork() {
  functionThatThrows();
}
}  // namespace

//=================================================================================================
/// Demonstrates usage and handling of exceptions
auto main() -> int {
  try {
    doWork();
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
