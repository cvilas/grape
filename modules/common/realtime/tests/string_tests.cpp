//=================================================================================================
// Copyright (C) 2023-2024 GRAPE Contributors
//=================================================================================================

#include <array>

#include "catch2/catch_test_macros.hpp"
#include "grape/realtime/fast_string.h"

namespace grape::realtime::tests {

// NOLINTBEGIN(cert-err58-cpp,cppcoreguidelines-avoid-magic-numbers)

using FastString8 = grape::realtime::FastString<7>;
using namespace std::string_literals;

//-------------------------------------------------------------------------------------------------
TEST_CASE("Default construction", "[FastString]") {
  constexpr FastString8 STR{};

  CHECK((0 == STR.length()));  // NOLINT(readability-container-size-empty)
  CHECK(STR.empty());
  CHECK((7 == STR.maxSize()));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("construction from constexpr", "[FastString]") {
  constexpr const char* STR = "1234";
  constexpr FastString8 FAST_STR = STR;

  CHECK((4 == FAST_STR.length()));
  CHECK_FALSE(FAST_STR.empty());
  CHECK((7 == FAST_STR.maxSize()));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("construction from const buffer", "[FastString]") {
  using FastString24 = grape::realtime::FastString<23>;
  constexpr std::array<char, 24> BUFFER{ 'H', 'e', 'l', 'l', 'o', ' ', 'W',
                                         'o', 'r', 'l', 'd', '!', '\0' };
  constexpr size_t BUFFER_STR_SIZE = 12;

  constexpr FastString24 STR(BUFFER.data(), BUFFER_STR_SIZE);

  CHECK(("Hello World!"s == STR.cStr()));
  CHECK((12 == STR.length()));
  CHECK_FALSE(STR.empty());
  CHECK((23 == STR.maxSize()));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("construction from buffer", "[FastString]") {
  using FastString24 = grape::realtime::FastString<23>;
  std::array<char, 24> buffer = {
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0'
  };
  size_t buffer_str_size = 12;

  FastString24 str(buffer.data(), buffer_str_size);

  CHECK(("Hello World!"s == str.cStr()));
  CHECK((12 == str.length()));
  CHECK_FALSE(str.empty());
  CHECK((23 == str.maxSize()));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("copy construction", "[FastString]") {
  constexpr FastString8 STR{ "abc" };
  auto str_copy = STR;

  CHECK((str_copy.length() == STR.length()));
  CHECK_FALSE(STR.empty());
  CHECK_FALSE(str_copy.empty());
  CHECK((str_copy.maxSize() == STR.maxSize()));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("move construction", "[FastString]") {
  constexpr FastString8 STR{ "abc" };
  auto str_copy = STR;
  auto str_move = std::move(str_copy);  // NOLINT(performance-move-const-arg)

  CHECK((str_move.length() == STR.length()));
  CHECK_FALSE(STR.empty());
  CHECK_FALSE(str_move.empty());
  CHECK((str_move.maxSize() == STR.maxSize()));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("copy assignment", "[FastString]") {
  constexpr FastString8 STR1{ "1234", 4 };
  FastString8 str2{ "lmnopqrstuvxyz" };

  CHECK(("1234"s == STR1.cStr()));
  CHECK(("lmnopqr"s == str2.cStr()));

  str2 = STR1;

  CHECK(("1234"s == str2.cStr()));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("move assignment", "[FastString]") {
  constexpr FastString8 STR1{ "1234", 4 };
  FastString8 str2{ "lmnopqrstuvxyz" };
  str2 = std::move(STR1);  // NOLINT(performance-move-const-arg)

  CHECK(("1234"s == str2.cStr()));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("using with string_view", "[FastString]") {
  static constexpr FastString8 STR{ "abcdefghij", 10 };
  constexpr auto STR_SUB_STR = STR.str().substr(0, 2);
  constexpr auto STR_STR = STR.str();
  constexpr auto STR_LENGTH = STR.length();

  CHECK(("abcdefg"s == STR.cStr()));
  CHECK(("ab"s == STR_SUB_STR));
  CHECK(("abcdefg"s == STR_STR));
  CHECK((7 == STR_LENGTH));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("comparison", "[FastString]") {
  constexpr FastString8 STR1{ "abcd" };
  constexpr FastString8 STR2{ "abcd" };
  constexpr FastString8 STR3{ "abcf" };

  CHECK((STR1 == STR2));
  CHECK_FALSE((STR2 == STR3));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("append", "[FastString]") {
  FastString8 str{ "abc" };
  CHECK(("abc"s == str.cStr()));

  str.append("d");
  CHECK(("abcd"s == str.cStr()));

  str.append("efghi", 5);
  CHECK(("abcdefg"s == str.cStr()));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("clear", "[FastString]") {
  FastString8 str{ "abcdefg" };
  CHECK(("abcdefg"s == str.cStr()));
  CHECK_FALSE(str.empty());

  str.clear();
  CHECK((""s == str.cStr()));
  CHECK(str.empty());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("swap", "[FastString]") {
  FastString8 str1{ "xyz" };
  FastString8 str2{ "34" };

  str2.swap(str1);
  CHECK(("xyz"s == str2.cStr()));
  CHECK(("34"s == str1.cStr()));

  std::swap(str2, str1);
  CHECK(("34"s == str2.cStr()));
  CHECK(("xyz"s == str1.cStr()));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("using member variables", "[FastString]") {
  // uses only 8 bytes in stack
  CHECK((8 == sizeof(FastString8)));
}

// NOLINTEND(cert-err58-cpp,cppcoreguidelines-avoid-magic-numbers)

}  // namespace grape::realtime::tests
