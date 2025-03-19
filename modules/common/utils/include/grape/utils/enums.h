//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

/// This header provides a basic implementation of static reflection for enums.
/// Implementation builds upon the idea presented here: https://godbolt.org/z/jx6vzPjnW as a
/// comment in the following discussion thread on Reddit:
/// https://www.reddit.com/r/cpp_questions/comments/1fdwn7x/meta_programming_magic_for_enum_names/
///
/// For advanced reflection support for enums
/// - Use conjure_enum (https://github.com/fix8mt/conjure_enum)
/// - Wait for C++26 when reflections become part of the standard!

#pragma once

#include <array>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

namespace grape::enums {
namespace detail {

/// Uses compiler-specific magic to extract a string representing an enumeration at compile-time
template <auto E>
consteval auto extractEnumeratorName() -> std::string_view {
  const std::string_view func_name = __PRETTY_FUNCTION__;

#if defined(__clang__)
  const std::string_view start_token = "[E = ";
  const std::string_view end_token = "]";
#elif defined(__GNUC__) || defined(__GNUG__)
  const std::string_view start_token = "with auto E = ";
  const std::string_view end_token = ";";
#else
#error "Unsupported compiler. Use GCC or Clang"
#endif
  const auto temp_args_list = func_name.substr(func_name.find(start_token) + start_token.size());
  if (temp_args_list.starts_with("(")) {
    return "";
  }

  const auto enum_name_end = temp_args_list.find(end_token);
  const auto enum_name_start = temp_args_list.rfind("::", enum_name_end) + 2;
  const auto enum_name_length = enum_name_end - enum_name_start;
  return temp_args_list.substr(enum_name_start, enum_name_length);
}

/// Wrapper that extracts enum names and puts them in static storage as uniquely named variables
template <auto E>
struct NameStorage {
  static constexpr auto NAME = extractEnumeratorName<E>();

  static constexpr auto makeArray() {
    std::array<char, NAME.size()> result{};
    for (std::size_t i = 0; i < NAME.size(); ++i) {
      result.at(i) = NAME.at(i);
    }
    return result;
  }

  static constexpr auto VALUE = makeArray();
};

template <auto E>
consteval auto extractAndAllocateEnumeratorName() -> std::string_view {
  return { NameStorage<E>::VALUE.data(), NameStorage<E>::VALUE.size() };
}

template <int N, int... Seq>
constexpr auto add(std::integer_sequence<int, Seq...> /*unused*/)
    -> std::integer_sequence<int, N + Seq...> {
  return {};
}

template <int Min, int Max>
using make_integer_range = decltype(add<Min>(std::make_integer_sequence<int, Max - Min>()));

}  // namespace detail

//-------------------------------------------------------------------------------------------------
/// Sets default range of values an enum can take. Users can override with a specialisation for the
/// type as follows (also see example programs)
///
/// enum class Color { Red = -2, Green, Blue, Black, White };
///
/// template <>
/// struct grape::enums::Range<Color> {
///   static constexpr int MIN = -2;
///   static constexpr int MAX = 2;
/// };
///
template <typename Enum>
  requires std::is_enum_v<Enum>
struct Range {
  static constexpr auto DEFAULT_MIN = 0;
  static constexpr auto DEFAULT_MAX = 7;
  static constexpr int MIN = DEFAULT_MIN;
  static constexpr int MAX = DEFAULT_MAX;
};

//-------------------------------------------------------------------------------------------------
/// Generates a list containing string representations for values in an enum type
template <typename Enum>
  requires std::is_enum_v<Enum>
static constexpr auto NAMES_LIST = ([]<auto... Es>(std::integer_sequence<int, Es...>) {
  return std::array{ detail::extractAndAllocateEnumeratorName<static_cast<Enum>(Es)>()... };
})(detail::make_integer_range<Range<Enum>::MIN, Range<Enum>::MAX + 1>{});

//-------------------------------------------------------------------------------------------------
/// Returns the string representation for a value in an enumeration
template <typename Enum>
  requires std::is_enum_v<Enum>
constexpr auto name(Enum val) -> std::string_view {
  const auto idx = static_cast<std::size_t>(static_cast<int>(val) - Range<Enum>::MIN);
  if (idx < NAMES_LIST<Enum>.size()) {
    return NAMES_LIST<Enum>.at(idx);
  }
  return "";
}

//-------------------------------------------------------------------------------------------------
// Returns the enumeration from string representation.
template <typename Enum>
  requires std::is_enum_v<Enum>
constexpr auto cast(std::string_view str_name) -> std::optional<Enum> {
  auto ival = std::underlying_type_t<Enum>{};
  for (const auto& name : NAMES_LIST<Enum>) {
    if (name == str_name) {
      // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
      return static_cast<Enum>(ival + Range<Enum>::MIN);
    }
    ++ival;
  }
  return std::nullopt;
}
}  // namespace grape::enums
