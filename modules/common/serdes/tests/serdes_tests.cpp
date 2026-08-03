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
using OutStream = grape::serdes::OutStream<BUF_SIZE>;
using InStream = grape::serdes::InStream;
using Serialiser = grape::serdes::Serialiser<OutStream>;
using Deserialiser = grape::serdes::Deserialiser<InStream>;

//-------------------------------------------------------------------------------------------------
TEST_CASE("Serialise arithmetic types", "[serdes]") {
  auto ostream = OutStream();
  auto ser = Serialiser(ostream);

  REQUIRE(ser.pack(std::int8_t{ 42 }));
  REQUIRE(ser.pack(std::uint16_t{ 1000 }));
  REQUIRE(ser.pack(std::int32_t{ -123456 }));
  REQUIRE(ser.pack(std::uint64_t{ 9876543210 }));
  REQUIRE(ser.pack(float{ 3.14F }));
  REQUIRE(ser.pack(double{ 2.71828 }));
  REQUIRE(ostream.size() == sizeof(std::int8_t) + sizeof(std::uint16_t) + sizeof(std::int32_t) +
                                sizeof(std::uint64_t) + sizeof(float) + sizeof(double));

  auto istream = InStream(ostream.data());
  auto des = Deserialiser(istream);

  std::int8_t i8{};
  std::uint16_t u16{};
  std::int32_t i32{};
  std::uint64_t u64{};
  float flt{};
  double dbl{};

  REQUIRE(des.unpack(i8));
  REQUIRE(des.unpack(u16));
  REQUIRE(des.unpack(i32));
  REQUIRE(des.unpack(u64));
  REQUIRE(des.unpack(flt));
  REQUIRE(des.unpack(dbl));

  REQUIRE(i8 == 42);
  REQUIRE(u16 == 1000);
  REQUIRE(i32 == -123456);
  REQUIRE(u64 == 9876543210);
  REQUIRE_THAT(static_cast<double>(flt), Catch::Matchers::WithinRel(3.14, 0.0001));
  REQUIRE_THAT(dbl, Catch::Matchers::WithinRel(2.71828, 0.0001));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Serialise string", "[serdes]") {
  auto ostream = OutStream();
  auto ser = Serialiser(ostream);

  REQUIRE(ser.pack(std::string{ "Hello, World!" }));
  REQUIRE(ostream.size() == sizeof(std::size_t) + 13);  // 13 is the length of "Hello, World!"

  auto istream = InStream(ostream.data());
  auto des = Deserialiser(istream);

  std::string str;
  REQUIRE(des.unpack(str));
  REQUIRE(str == "Hello, World!");
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Serialise vector", "[serdes]") {
  auto ostream = OutStream();
  auto ser = Serialiser(ostream);

  std::vector<int> vec{ 1, 2, 3, 4, 5 };
  REQUIRE(ser.pack(vec));
  REQUIRE(ostream.size() == sizeof(std::size_t) + (vec.size() * sizeof(int)));

  auto istream = InStream(ostream.data());
  auto des = Deserialiser(istream);

  std::vector<int> deserialized_vec;
  REQUIRE(des.unpack(deserialized_vec));
  REQUIRE(deserialized_vec == vec);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Serialise array", "[serdes]") {
  auto ostream = OutStream();
  auto ser = Serialiser(ostream);

  std::array<double, 3> arr{ 1.1, 2.2, 3.3 };
  REQUIRE(ser.pack(arr));
  REQUIRE(ostream.size() == arr.size() * sizeof(double));

  auto istream = InStream(ostream.data());
  auto des = Deserialiser(istream);

  std::array<double, 3> deserialized_arr{};
  REQUIRE(des.unpack(deserialized_arr));
  REQUIRE(deserialized_arr == arr);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Serialise variant", "[serdes]") {
  using Var = std::variant<int, float, std::string>;

  auto ostream = OutStream();
  auto ser = Serialiser(ostream);

  // Test with int
  const auto v1 = Var{ 42 };
  REQUIRE(ser.pack(v1));
  REQUIRE(ostream.size() > 0);

  auto istream = InStream(ostream.data());
  auto des = Deserialiser(istream);

  Var v2;
  REQUIRE(des.unpack(v2));
  REQUIRE(v1.index() == v2.index());
  REQUIRE(std::get<int>(v2) == 42);

  // Test with float
  ostream.reset();
  const auto v3 = Var{ 3.14F };
  REQUIRE(ser.pack(v3));
  istream = InStream(ostream.data());
  Var v4;
  REQUIRE(des.unpack(v4));
  REQUIRE(v3.index() == v4.index());
  REQUIRE_THAT(std::get<float>(v4), Catch::Matchers::WithinRel(3.14F, 0.0001F));

  // Test with string
  ostream.reset();
  const auto v5 = Var{ std::string("hello") };
  REQUIRE(ser.pack(v5));
  istream = InStream(ostream.data());
  Var v6;
  REQUIRE(des.unpack(v6));
  REQUIRE(v5.index() == v6.index());
  REQUIRE(std::get<std::string>(v6) == "hello");
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Serialise chrono time_point", "[serdes]") {
  using TimePoint = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>;

  auto ostream = OutStream();
  auto ser = Serialiser(ostream);

  const auto t1 = TimePoint{ std::chrono::nanoseconds{ 123456789 } };
  REQUIRE(ser.pack(t1));

  auto istream = InStream(ostream.data());
  auto des = Deserialiser(istream);

  auto t2 = TimePoint{};
  REQUIRE(des.unpack(t2));
  REQUIRE(t2 == t1);
}

//-------------------------------------------------------------------------------------------------
enum class MotorMode : std::uint8_t {
  Idle = 0,
  Position = 1,
  Velocity = 2,
};

//-------------------------------------------------------------------------------------------------
TEST_CASE("Serialise enum", "[serdes]") {
  auto ostream = OutStream();
  auto ser = Serialiser(ostream);

  constexpr auto MODE1 = MotorMode::Velocity;
  REQUIRE(ser.pack(MODE1));
  REQUIRE(ostream.size() == sizeof(std::underlying_type_t<MotorMode>));

  auto istream = InStream(ostream.data());
  auto des = Deserialiser(istream);

  auto mode2 = MotorMode::Idle;
  REQUIRE(des.unpack(mode2));
  REQUIRE(mode2 == MODE1);
}

//-------------------------------------------------------------------------------------------------
struct Point {
  int x{ 0 };
  int y{ 0 };
  auto operator==(const Point&) const -> bool = default;
};

//-------------------------------------------------------------------------------------------------
TEST_CASE("Serialise custom data structure", "[serdes]") {
  auto ostream = OutStream();
  auto ser = Serialiser(ostream);

  const auto p1 = Point{ .x = 42, .y = -7 };
  REQUIRE(ser.pack(p1));
  REQUIRE(ostream.size() == 2 * sizeof(int));

  auto istream = InStream(ostream.data());
  auto des = Deserialiser(istream);

  Point p2;
  REQUIRE(des.unpack(p2));
  REQUIRE(p2 == p1);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace
