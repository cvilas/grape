//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <array>

#include "catch2/catch_test_macros.hpp"
#include "grape/realtime/fixed_string.h"

namespace {

// NOLINTBEGIN(cert-err58-cpp,cppcoreguidelines-avoid-magic-numbers)

using FixedString8 = grape::realtime::FixedString<7>;
using namespace std::string_literals;

//-------------------------------------------------------------------------------------------------
TEST_CASE("Default constructed string is empty", "[FixedString]") {
  constexpr FixedString8 STR{};

  CHECK(0 == STR.length());  // NOLINT(readability-container-size-empty)
  CHECK(STR.empty());
  CHECK(7 == STR.maxSize());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Construction from constexpr string", "[FixedString]") {
  constexpr const char* STR = "1234";
  constexpr FixedString8 FAST_STR(STR);

  CHECK(4 == FAST_STR.length());
  CHECK_FALSE(FAST_STR.empty());
  CHECK(7 == FAST_STR.maxSize());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Construction from const buffer", "[FixedString]") {
  using FixedString24 = grape::realtime::FixedString<23>;
  constexpr std::array<char, 24> BUFFER{ 'H', 'e', 'l', 'l', 'o', ' ', 'W',
                                         'o', 'r', 'l', 'd', '!', '\0' };

  constexpr FixedString24 STR(BUFFER.data());

  CHECK("Hello World!"s == STR.cStr());
  CHECK(12 == STR.length());
  CHECK_FALSE(STR.empty());
  CHECK(23 == STR.maxSize());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Copy construction", "[FixedString]") {
  constexpr FixedString8 STR{ "abc" };
  auto str_copy = STR;

  CHECK(str_copy.length() == STR.length());
  CHECK_FALSE(STR.empty());
  CHECK_FALSE(str_copy.empty());
  CHECK(str_copy.maxSize() == STR.maxSize());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Copy assignment", "[FixedString]") {
  constexpr FixedString8 STR1{ "1234" };
  FixedString8 str2{ "lmnopqrstuvxyz" };

  CHECK("1234"s == STR1.cStr());
  CHECK("lmnopqr"s == str2.cStr());

  str2 = STR1;

  CHECK("1234"s == str2.cStr());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Using with string_view", "[FixedString]") {
  static constexpr FixedString8 STR{ "abcdefghij" };
  constexpr auto STR_SUB_STR = STR.str().substr(0, 2);
  constexpr auto STR_STR = STR.str();
  constexpr auto STR_LENGTH = STR.length();

  CHECK("abcdefg"s == STR.cStr());
  CHECK("ab"s == STR_SUB_STR);
  CHECK("abcdefg"s == STR_STR);
  CHECK(7 == STR_LENGTH);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Comparison", "[FixedString]") {
  constexpr FixedString8 STR1{ "abcd" };
  constexpr FixedString8 STR2{ "abcd" };
  constexpr FixedString8 STR3{ "abcf" };

  CHECK(STR1 == STR2);
  CHECK_FALSE(STR2 == STR3);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Append", "[FixedString]") {
  FixedString8 str{ "abc" };
  CHECK("abc"s == str.cStr());

  str.append("d");
  CHECK("abcd"s == str.cStr());

  str.append("efghi", 5);
  CHECK("abcdefg"s == str.cStr());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Clear", "[FixedString]") {
  FixedString8 str{ "abcdefg" };
  CHECK("abcdefg"s == str.cStr());
  CHECK_FALSE(str.empty());

  str.clear();
  CHECK(""s == str.cStr());
  CHECK(str.empty());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Swap", "[FixedString]") {
  FixedString8 str1{ "xyz" };
  FixedString8 str2{ "34" };

  str2.swap(str1);
  CHECK("xyz"s == str2.cStr());
  CHECK("34"s == str1.cStr());

  std::swap(str2, str1);
  CHECK("34"s == str2.cStr());
  CHECK("xyz"s == str1.cStr());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Size in stack", "[FixedString]") {
  CHECK(8 == sizeof(FixedString8));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Construct with format string", "[FixedString]") {
  SECTION("Format string fits within capacity") {
    grape::realtime::FixedString<20> str("{} + {} = {}", 2, 3, 5);
    REQUIRE(std::string_view(str.data()) == "2 + 3 = 5");
    REQUIRE(str.length() == 9);
  }

  SECTION("Format string exceeds capacity") {
    grape::realtime::FixedString<10> str("Long string: {}", "too long");
    REQUIRE(std::string_view(str.data()) == "Long strin");
    REQUIRE(str.length() == 10);
  }

  SECTION("Empty format string") {
    grape::realtime::FixedString<5> str("", 42);
    REQUIRE(std::string_view(str.data()).empty());
    REQUIRE(str.length() == 0);
  }

  SECTION("Format string with multiple types") {
    grape::realtime::FixedString<30> str("{} {} {:.2f}", "Answer:", 42, 1.23456);
    REQUIRE(std::string_view(str.data()) == "Answer: 42 1.23");
    REQUIRE(str.length() == 15);
  }
}

// NOLINTEND(cert-err58-cpp,cppcoreguidelines-avoid-magic-numbers)

}  // namespace
