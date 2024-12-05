//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "catch2/catch_test_macros.hpp"
#include "grape/utils/enums.h"

// NOLINTBEGIN(clang-analyzer-optin.core.EnumCastOutOfRange)

// Test-case enumeration with customised range
enum class Color { Red = -2, Green = 0, Blue = 1, Black = 2, White = 4 };

template <>
struct grape::enums::enum_range<Color> {
  static constexpr int min = -2;
  static constexpr int max = 4;
};

namespace {
//-------------------------------------------------------------------------------------------------
TEST_CASE("Enum names are extracted correctly", "[enums]") {
  REQUIRE(grape::enums::enum_name(Color::Red) == "Red");
  REQUIRE(grape::enums::enum_name(Color::Green) == "Green");
  REQUIRE(grape::enums::enum_name(Color::Blue) == "Blue");
  REQUIRE(grape::enums::enum_name(Color::Black) == "Black");
  REQUIRE(grape::enums::enum_name(Color::White) == "White");
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Values outside enum range return empty string", "[enums]") {
  REQUIRE(grape::enums::enum_name(static_cast<Color>(-3)).empty());
  REQUIRE(grape::enums::enum_name(static_cast<Color>(5)).empty());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Invalid values within enum range return empty string", "[enums]") {
  REQUIRE(grape::enums::enum_name(static_cast<Color>(-1)).empty());
  REQUIRE(grape::enums::enum_name(static_cast<Color>(3)).empty());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Valid string names can be cast back to enum", "[enums]") {
  auto maybe_color = grape::enums::enum_cast<Color>("Red");
  REQUIRE((maybe_color.has_value() and (maybe_color.value() == Color::Red)));

  maybe_color = grape::enums::enum_cast<Color>("Green");
  REQUIRE((maybe_color.has_value() and (maybe_color.value() == Color::Green)));

  maybe_color = grape::enums::enum_cast<Color>("Blue");
  REQUIRE((maybe_color.has_value() and (maybe_color.value() == Color::Blue)));

  maybe_color = grape::enums::enum_cast<Color>("Black");
  REQUIRE((maybe_color.has_value() and (maybe_color.value() == Color::Black)));

  maybe_color = grape::enums::enum_cast<Color>("White");
  REQUIRE((maybe_color.has_value() and (maybe_color.value() == Color::White)));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Invalid string name can be cast back to enum", "[enums]") {
  const auto maybe_color = grape::enums::enum_cast<Color>("Red--");
  REQUIRE(not maybe_color.has_value());
}

}  // namespace

// NOLINTEND(clang-analyzer-optin.core.EnumCastOutOfRange)
