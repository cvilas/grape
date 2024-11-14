//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <cstdlib>
#include <print>

#include "grape/utils/enums.h"

//=================================================================================================
// Example program demonstrates usage of facilities for static reflection of enums
//=================================================================================================

/// An example enumeration
enum class Color { Red = -2, Green, Blue, Black, White };

/// Specialisation for the enumeration. Necessary if the enumeration values are not in the default
/// range [enum_range::DEFAULT_MIN, enum_range::DEFAULT_MAX]
template <>
struct grape::enums::enum_range<Color> {
  static constexpr int min = -2;
  static constexpr int max = 2;
};

//=================================================================================================
auto main() -> int {
  try {
    /// String representation of a specific enumeration value can be obtained as follows:
    std::println("{}", grape::enums::enum_name(Color::Red));

    /// Here's another way
    auto value = static_cast<std::underlying_type_t<Color>>(Color::Red);
    value += 2;
    std::println("{}", grape::enums::enum_name(static_cast<Color>(value)));

    /// A table of all values in the enumeration as strings
    std::println("All values: {}", grape::enums::enum_names_list<Color>);

    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}
