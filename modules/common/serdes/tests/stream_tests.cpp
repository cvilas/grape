//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "catch2/catch_test_macros.hpp"
#include "grape/serdes/stream.h"

namespace {

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

//-------------------------------------------------------------------------------------------------
TEST_CASE("OutStream functionality", "[OutStream]") {
  grape::serdes::OutStream<10> out;

  SECTION("Initial state") {
    REQUIRE(out.size() == 0);
    REQUIRE(out.capacity() == 10);
  }

  SECTION("Write data") {
    REQUIRE(out.write("Hello", 5));
    REQUIRE(out.size() == 5);
    REQUIRE(std::string_view(out.data(), 5) == "Hello");
  }

  SECTION("Write beyond capacity") {
    REQUIRE(out.write("HelloWorld", 10));
    REQUIRE_FALSE(out.write("!", 1));
    REQUIRE(out.size() == 10);
  }

  SECTION("Rewind and reset") {
    REQUIRE(out.write("HelloWorld", 10));
    out.rewind(5);
    REQUIRE(out.size() == 5);
    out.reset();
    REQUIRE(out.size() == 0);
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("InStream functionality", "[InStream]") {
  std::string data = "HelloWorld";
  grape::serdes::InStream in(std::span(data.data(), data.size()));

  SECTION("Initial state") {
    REQUIRE(in.size() == 10);
  }

  SECTION("Read data") {
    std::vector<char> buffer(6, '\0');
    REQUIRE(in.read(buffer.data(), 5));
    REQUIRE(std::string_view(buffer.data(), 5) == "Hello");
  }

  SECTION("Read beyond available data") {
    std::vector<char> buffer(11, '\0');
    REQUIRE(in.read(buffer.data(), 10));
    REQUIRE_FALSE(in.read(buffer.data(), 1));
  }

  SECTION("Rewind and reset") {
    std::vector<char> buffer(6, '\0');
    REQUIRE(in.read(buffer.data(), 5));
    in.rewind(3);
    REQUIRE(in.read(buffer.data(), 3));
    REQUIRE(std::string_view(buffer.data(), 3) == "llo");
    in.reset();
    REQUIRE(in.read(buffer.data(), 5));
    REQUIRE(std::string_view(buffer.data(), 5) == "Hello");
  }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace
