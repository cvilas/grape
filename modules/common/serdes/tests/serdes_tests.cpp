//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include "grape/serdes/serdes.h"
#include "grape/serdes/stream.h"

namespace {

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

constexpr auto BUF_SIZE = 1024U;

//-------------------------------------------------------------------------------------------------
TEST_CASE("Serialize arithmetic types", "[serdes]") {
  auto ostream = grape::serdes::OutStream<BUF_SIZE>();
  auto ser = grape::serdes::Serialiser(ostream);

  REQUIRE(ser.pack(std::int8_t{ 42 }));
  REQUIRE(ser.pack(std::uint16_t{ 1000 }));
  REQUIRE(ser.pack(std::int32_t{ -123456 }));
  REQUIRE(ser.pack(std::uint64_t{ 9876543210 }));
  REQUIRE(ser.pack(float{ 3.14F }));
  REQUIRE(ser.pack(double{ 2.71828 }));
  REQUIRE(ostream.size() == sizeof(std::int8_t) + sizeof(std::uint16_t) + sizeof(std::int32_t) +
                                sizeof(std::uint64_t) + sizeof(float) + sizeof(double));

  auto istream = grape::serdes::InStream({ ostream.data(), ostream.size() });
  auto des = grape::serdes::Deserialiser(istream);

  std::int8_t i8{};
  std::uint16_t u16{};
  std::int32_t i32{};
  std::uint64_t u64{};
  float f{};
  double d{};

  REQUIRE(des.unpack(i8));
  REQUIRE(des.unpack(u16));
  REQUIRE(des.unpack(i32));
  REQUIRE(des.unpack(u64));
  REQUIRE(des.unpack(f));
  REQUIRE(des.unpack(d));

  REQUIRE(i8 == 42);
  REQUIRE(u16 == 1000);
  REQUIRE(i32 == -123456);
  REQUIRE(u64 == 9876543210);
  REQUIRE_THAT(static_cast<double>(f), Catch::Matchers::WithinRel(3.14, 0.0001));
  REQUIRE_THAT(d, Catch::Matchers::WithinRel(2.71828, 0.0001));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Serialize string", "[serdes]") {
  auto ostream = grape::serdes::OutStream<BUF_SIZE>();
  auto ser = grape::serdes::Serialiser(ostream);

  REQUIRE(ser.pack(std::string{ "Hello, World!" }));
  REQUIRE(ostream.size() == sizeof(std::uint32_t) + 13);  // 13 is the length of "Hello, World!"

  auto istream = grape::serdes::InStream({ ostream.data(), ostream.size() });
  auto des = grape::serdes::Deserialiser(istream);

  std::string str;
  REQUIRE(des.unpack(str));
  REQUIRE(str == "Hello, World!");
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Serialize vector", "[serdes]") {
  auto ostream = grape::serdes::OutStream<BUF_SIZE>();
  auto ser = grape::serdes::Serialiser(ostream);

  std::vector<int> vec{ 1, 2, 3, 4, 5 };
  REQUIRE(ser.pack(vec));
  REQUIRE(ostream.size() == sizeof(std::uint32_t) + (vec.size() * sizeof(int)));

  auto istream = grape::serdes::InStream({ ostream.data(), ostream.size() });
  auto des = grape::serdes::Deserialiser(istream);

  std::vector<int> deserialized_vec;
  REQUIRE(des.unpack(deserialized_vec));
  REQUIRE(deserialized_vec == vec);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Serialize array", "[serdes]") {
  auto ostream = grape::serdes::OutStream<BUF_SIZE>();
  auto ser = grape::serdes::Serialiser(ostream);

  std::array<double, 3> arr{ 1.1, 2.2, 3.3 };
  REQUIRE(ser.pack(arr));
  REQUIRE(ostream.size() == arr.size() * sizeof(double));

  auto istream = grape::serdes::InStream({ ostream.data(), ostream.size() });
  auto des = grape::serdes::Deserialiser(istream);

  std::array<double, 3> deserialized_arr{};
  REQUIRE(des.unpack(deserialized_arr));
  REQUIRE(deserialized_arr == arr);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace
