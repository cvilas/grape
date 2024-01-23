//=================================================================================================
// Copyright (C) 2023-2024 GRAPE Contributors
//=================================================================================================

#include <print>

#include "grape/realtime/fast_string.h"

//=================================================================================================
auto main() -> int {
  try {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    using FastString8 = grape::realtime::FastString<7>;

    static constexpr FastString8 STRING = "abcdefghij";  //!< this will get truncated to fit
    constexpr auto SUB_STRING = STRING.str().substr(0, 2);
    constexpr auto STRING_LENGTH = STRING.length();
    std::println("String='{}', length={}", STRING.str(), STRING_LENGTH);
    std::println("Sub-string='{}'", SUB_STRING);
  } catch (const std::exception& ex) {
    std::ignore = fputs(ex.what(), stderr);
  }
  return EXIT_SUCCESS;
}