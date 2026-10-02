//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <algorithm>
#include <span>
#include <string>

#include "catch2/catch_test_macros.hpp"
#include "grape/serdes/stream.h"

namespace {

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

[[nodiscard]] constexpr auto toSpan(const std::string& str) -> std::span<const std::byte> {
  return std::as_bytes(std::span{ str });
}

[[nodiscard]] constexpr auto toSpan(std::string& str) -> std::span<std::byte> {
  return std::as_writable_bytes(std::span{ str });
}

[[nodiscard]] auto toString(std::span<const std::byte> bytes) -> std::string {
  auto str = std::string(bytes.size(), '\0');
  std::ranges::copy(bytes, std::as_writable_bytes(std::span{ str }).begin());
  return str;
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("OutStream functionality", "[OutStream]") {
  grape::serdes::OutStream<10> out;

  SECTION("Initial state") {
    REQUIRE(out.size() == 0);
    REQUIRE(out.capacity() == 10);
  }

  SECTION("Write data") {
    REQUIRE(out.write(toSpan("Hello")));
    REQUIRE(out.size() == 5);
    REQUIRE(toString(out.data()) == "Hello");
  }

  SECTION("Write beyond capacity") {
    REQUIRE(out.write(toSpan("HelloWorld")));
    REQUIRE_FALSE(out.write(toSpan("!")));
    REQUIRE(out.size() == 10);
  }

  SECTION("Rewind and reset") {
    REQUIRE(out.write(toSpan("HelloWorld")));
    out.rewind(5);
    REQUIRE(out.size() == 5);
    out.reset();
    REQUIRE(out.size() == 0);
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("InStream functionality", "[InStream]") {
  auto istream = std::string("HelloWorld");
  grape::serdes::InStream in(toSpan(istream));

  SECTION("Initial state") {
    REQUIRE(in.size() == 10);
  }

  SECTION("Read data") {
    std::string str(5, '\0');
    REQUIRE(in.read(toSpan(str)));
    REQUIRE(str == "Hello");
  }

  SECTION("Read beyond available data") {
    std::string buffer(10, '\0');
    REQUIRE(in.read(toSpan(buffer)));
    std::string buffer2(1, '\0');
    REQUIRE_FALSE(in.read(toSpan(buffer2)));
  }

  SECTION("Rewind and reset") {
    std::string buffer(5, '\0');
    REQUIRE(in.read(toSpan(buffer)));
    in.rewind(3);
    std::string buffer2(3, '\0');
    REQUIRE(in.read(toSpan(buffer2)));
    REQUIRE(buffer2 == "llo");
    in.reset();
    std::string buffer3(5, '\0');
    REQUIRE(in.read(toSpan(buffer3)));
    REQUIRE(buffer3 == "Hello");
  }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace
