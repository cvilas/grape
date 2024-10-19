//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <print>

#include "grape/exception.h"
#include "grape/realtime/fixed_string.h"

//=================================================================================================
auto main() -> int {
  try {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    using FixedString8 = grape::realtime::FixedString<7>;

    static constexpr FixedString8 STRING = "abcdefghij";  //!< this will get truncated to fit
    constexpr auto SUB_STRING = STRING.str().substr(0, 2);
    constexpr auto STRING_LENGTH = STRING.length();
    std::println("String='{}', length={}", STRING.str(), STRING_LENGTH);
    std::println("Sub-string='{}'", SUB_STRING);
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
