//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <expected>
#include <filesystem>
#include <memory>

#include "grape/error.h"
#include "grape/wall_clock.h"

namespace grape::rpi::sense_hat {

/// Raw 3-axis sample
template <typename T>
struct Vec3 {
  T x{};
  T y{};
  T z{};
};

/// Raw IMU sample
struct ImuSample {
  WallClock::TimePoint timestamp;  //!< Sample timestamp
  Vec3<std::int16_t> gyro{};       //!< Raw gyroscope counts
  Vec3<std::int16_t> accel{};      //!< Raw accelerometer counts
  Vec3<std::int16_t> mag{};        //!< Raw magnetometer counts (updated at mag rate)
  bool mag_updated{ false };       //!< True if mag was read this tick
};

static_assert(std::is_trivially_copyable_v<ImuSample>);

//=================================================================================================
/// Interface to the inertial measurement unit in the Sense HAT v2
///
class InertialSensor {
public:
  struct Config {
    std::filesystem::path i2c_bus{ "/dev/i2c-1" };
  };

  explicit InertialSensor(const Config& config);
  [[nodiscard]] auto read() const -> std::expected<ImuSample, Error>;

  ~InertialSensor();
  InertialSensor(const InertialSensor&) = delete;
  InertialSensor(InertialSensor&&) = delete;
  auto operator=(const InertialSensor&) = delete;
  auto operator=(InertialSensor&&) = delete;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace grape::rpi::sense_hat
