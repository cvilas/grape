//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <array>

#include "catch2/catch_test_macros.hpp"
#include "grape/realtime/fixed_string.h"

namespace grape::realtime::tests {

// NOLINTBEGIN(cert-err58-cpp,cppcoreguidelines-avoid-magic-numbers)

using FixedString8 = grape::realtime::FixedString<7>;
using namespace std::string_literals;

//-------------------------------------------------------------------------------------------------
TEST_CASE("Default construction", "[FixedString]") {
  constexpr FixedString8 STR{};

  CHECK((0 == STR.length()));  // NOLINT(readability-container-size-empty)
  CHECK(STR.empty());
  CHECK((7 == STR.maxSize()));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("construction from constexpr", "[FixedString]") {
  constexpr const char* STR = "1234";
  constexpr FixedString8 FAST_STR = STR;

  CHECK((4 == FAST_STR.length()));
  CHECK_FALSE(FAST_STR.empty());
  CHECK((7 == FAST_STR.maxSize()));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("construction from const buffer", "[FixedString]") {
  using FixedString24 = grape::realtime::FixedString<23>;
  constexpr std::array<char, 24> BUFFER{ 'H', 'e', 'l', 'l', 'o', ' ', 'W',
                                         'o', 'r', 'l', 'd', '!', '\0' };
  constexpr size_t BUFFER_STR_SIZE = 12;

  constexpr FixedString24 STR(BUFFER.data(), BUFFER_STR_SIZE);

  CHECK(("Hello World!"s == STR.cStr()));
  CHECK((12 == STR.length()));
  CHECK_FALSE(STR.empty());
  CHECK((23 == STR.maxSize()));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("construction from buffer", "[FixedString]") {
  using FixedString24 = grape::realtime::FixedString<23>;
  std::array<char, 24> buffer = {
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0'
  };
  size_t buffer_str_size = 12;

  FixedString24 str(buffer.data(), buffer_str_size);

  CHECK(("Hello World!"s == str.cStr()));
  CHECK((12 == str.length()));
  CHECK_FALSE(str.empty());
  CHECK((23 == str.maxSize()));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("copy construction", "[FixedString]") {
  constexpr FixedString8 STR{ "abc" };
  auto str_copy = STR;

  CHECK((str_copy.length() == STR.length()));
  CHECK_FALSE(STR.empty());
  CHECK_FALSE(str_copy.empty());
  CHECK((str_copy.maxSize() == STR.maxSize()));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("move construction", "[FixedString]") {
  constexpr FixedString8 STR{ "abc" };
  auto str_copy = STR;
  auto str_move = std::move(str_copy);  // NOLINT(performance-move-const-arg)

  CHECK((str_move.length() == STR.length()));
  CHECK_FALSE(STR.empty());
  CHECK_FALSE(str_move.empty());
  CHECK((str_move.maxSize() == STR.maxSize()));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("copy assignment", "[FixedString]") {
  constexpr FixedString8 STR1{ "1234", 4 };
  FixedString8 str2{ "lmnopqrstuvxyz" };

  CHECK(("1234"s == STR1.cStr()));
  CHECK(("lmnopqr"s == str2.cStr()));

  str2 = STR1;

  CHECK(("1234"s == str2.cStr()));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("move assignment", "[FixedString]") {
  constexpr FixedString8 STR1{ "1234", 4 };
  FixedString8 str2{ "lmnopqrstuvxyz" };
  str2 = std::move(STR1);  // NOLINT(performance-move-const-arg)

  CHECK(("1234"s == str2.cStr()));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("using with string_view", "[FixedString]") {
  static constexpr FixedString8 STR{ "abcdefghij", 10 };
  constexpr auto STR_SUB_STR = STR.str().substr(0, 2);
  constexpr auto STR_STR = STR.str();
  constexpr auto STR_LENGTH = STR.length();

  CHECK(("abcdefg"s == STR.cStr()));
  CHECK(("ab"s == STR_SUB_STR));
  CHECK(("abcdefg"s == STR_STR));
  CHECK((7 == STR_LENGTH));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("comparison", "[FixedString]") {
  constexpr FixedString8 STR1{ "abcd" };
  constexpr FixedString8 STR2{ "abcd" };
  constexpr FixedString8 STR3{ "abcf" };

  CHECK((STR1 == STR2));
  CHECK_FALSE((STR2 == STR3));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("append", "[FixedString]") {
  FixedString8 str{ "abc" };
  CHECK(("abc"s == str.cStr()));

  str.append("d");
  CHECK(("abcd"s == str.cStr()));

  str.append("efghi", 5);
  CHECK(("abcdefg"s == str.cStr()));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("clear", "[FixedString]") {
  FixedString8 str{ "abcdefg" };
  CHECK(("abcdefg"s == str.cStr()));
  CHECK_FALSE(str.empty());

  str.clear();
  CHECK((""s == str.cStr()));
  CHECK(str.empty());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("swap", "[FixedString]") {
  FixedString8 str1{ "xyz" };
  FixedString8 str2{ "34" };

  str2.swap(str1);
  CHECK(("xyz"s == str2.cStr()));
  CHECK(("34"s == str1.cStr()));

  std::swap(str2, str1);
  CHECK(("34"s == str2.cStr()));
  CHECK(("xyz"s == str1.cStr()));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("using member variables", "[FixedString]") {
  // uses only 8 bytes in stack
  CHECK((8 == sizeof(FixedString8)));
}

// NOLINTEND(cert-err58-cpp,cppcoreguidelines-avoid-magic-numbers)

}  // namespace grape::realtime::tests
