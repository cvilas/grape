//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <print>

#include "grape/exception.h"
#include "grape/realtime/fixed_string.h"

//=================================================================================================
auto main() -> int {
  try {
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
    using FixedString8 = grape::realtime::FixedString<7>;
    using FixedString64 = grape::realtime::FixedString<63>;
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

    static constexpr FixedString8 STRING("abcdefghij");  //!< this will get truncated to fit
    constexpr auto SUB_STRING = STRING.str().substr(0, 2);
    constexpr auto STRING_LENGTH = STRING.length();
    std::println("String='{}', length={}", STRING.str(), STRING_LENGTH);
    std::println("Sub-string='{}'", SUB_STRING);

    const FixedString64 string64{ "{} + {} = {}", 2, 3, 5 };
    std::println("{}", string64.str());
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
