//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <cstdlib>
#include <print>

#include "grape/utils/enums.h"
#include "grape/utils/format_ranges.h"

//=================================================================================================
// Example program demonstrates usage of facilities for static reflection of enums
//=================================================================================================

/// An example enumeration
enum class Color : std::int8_t { Red = -2, Green, Blue, Black, White };

/// Specialisation for the enumeration. Necessary if the enumeration values are not in the default
/// range [Range::DEFAULT_MIN, Range::DEFAULT_MAX]
template <>
struct grape::enums::Range<Color> {
  static constexpr int MIN = -2;
  static constexpr int MAX = 2;
};

//=================================================================================================
auto main() -> int {
  try {
    /// String representation of a specific enumeration value can be obtained as follows:
    std::println("{}", grape::enums::name(Color::Red));

    /// Here's another way
    auto value = static_cast<std::underlying_type_t<Color>>(Color::Red);
    value += 2;
    std::println("{}", grape::enums::name(static_cast<Color>(value)));

    /// A table of all values in the enumeration as strings
    std::println("All values: {}", grape::enums::NAMES_LIST<Color>);

    /// Get enumeration from string
    const auto* enum_str = "Blue";
    const auto maybe_enum = grape::enums::cast<Color>(enum_str);
    if (maybe_enum.has_value()) {
      std::println("'{}' -> '{}'", enum_str, grape::enums::name(maybe_enum.value()));
    } else {
      std::println("Cannot convert string '{}' to valid enum of type Color", enum_str);
    }

    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}
