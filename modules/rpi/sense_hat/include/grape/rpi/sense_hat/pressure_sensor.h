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

//=================================================================================================
/// Interface to the pressure sensor in the Sense HAT v2
///
class PressureSensor {
public:
  struct Config {
    std::filesystem::path i2c_bus{ "/dev/i2c-1" };
  };

  struct Measurement {
    WallClock::TimePoint timestamp;
    float pressure_hpa{ 0.F };
    float temperature_celsius{ 0.F };
  };

  explicit PressureSensor(const Config& config);
  [[nodiscard]] auto read() -> std::expected<Measurement, Error>;

  ~PressureSensor();
  PressureSensor(const PressureSensor&) = delete;
  PressureSensor(PressureSensor&&) = delete;
  auto operator=(const PressureSensor&) = delete;
  auto operator=(PressureSensor&&) = delete;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace grape::rpi::sense_hat
