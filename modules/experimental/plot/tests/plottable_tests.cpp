//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <chrono>
#include <cstdint>

#include <catch2/catch_test_macros.hpp>

#include "grape/plot/plottable.h"

struct SensorData {
  std::chrono::steady_clock::time_point timestamp;
  double temperature{};
  std::int32_t pressure{};
  float humidity{};
};
static_assert(grape::Plottable<SensorData>);

struct SimpleData {
  std::chrono::system_clock::time_point timestamp;
  int value{};
};
static_assert(grape::Plottable<SimpleData>);

struct ImuData {
  std::chrono::high_resolution_clock::time_point timestamp;
  float accel_x{};
  float accel_y{};
  float accel_z{};
  float gyro_x{};
  float gyro_y{};
  float gyro_z{};
};
static_assert(grape::Plottable<ImuData>);

struct NoTimestamp {
  double value{};
};
static_assert(!grape::Plottable<NoTimestamp>);

struct WrongTimestampType {
  int timestamp{};
  double value{};
};
static_assert(!grape::Plottable<WrongTimestampType>);

struct DurationTimestampType {
  std::chrono::nanoseconds timestamp{};
  double value{};
};
static_assert(!grape::Plottable<DurationTimestampType>);

static_assert(!grape::Plottable<int>);
static_assert(!grape::Plottable<double>);

//-------------------------------------------------------------------------------------------------
TEST_CASE("Dummy", "[Plottable]") {
  WARN("No runtime tests implemented");
}
