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
/// Interface to the humidity sensor in the Sense HAT v2
///
class HumiditySensor {
public:
  struct Config {
    std::filesystem::path i2c_bus{ "/dev/i2c-1" };
  };

  struct Measurement {
    WallClock::TimePoint timestamp;
    float relative_humidity_percent{ 0.0 };
    float temperature_celsius{ 0.0 };
  };

  explicit HumiditySensor(const Config& config);
  [[nodiscard]] auto read() -> std::expected<Measurement, Error>;

  ~HumiditySensor();
  HumiditySensor(const HumiditySensor&) = delete;
  HumiditySensor(HumiditySensor&&) = delete;
  auto operator=(const HumiditySensor&) = delete;
  auto operator=(HumiditySensor&&) = delete;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace grape::rpi::sense_hat
