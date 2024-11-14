//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
// MIT License
//=================================================================================================

#include <format>

#include "grape/exception.h"
#include "grape/utils/enums.h"

namespace {
enum class Error : uint8_t { Bad, RealBad };

void functionThatThrows() {
  grape::panic<grape::Exception>(
      std::format("Boom!! [{}]", grape::enums::enum_name(Error::RealBad)));
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
