//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/rpi/sense_hat/pressure_sensor.h"

#include <array>
#include <format>
#include <thread>

#include "grape/exception.h"
#include "lps25h.h"
#include "utils.h"

namespace grape::rpi::sense_hat {

//=================================================================================================
struct PressureSensor::Impl {
  explicit Impl(I2CBus i2c_bus) : bus(std::move(i2c_bus)) {
  }
  I2CBus bus;
};

//-------------------------------------------------------------------------------------------------
PressureSensor::~PressureSensor() {
  if (impl_ != nullptr) {
    std::ignore = impl_->bus.write(lps25h::ADDR, lps25h::CTRL_REG1,
                                   std::array<std::uint8_t, 1>{ lps25h::POWER_DOWN });
  }
}

//-------------------------------------------------------------------------------------------------
PressureSensor::PressureSensor(const Config& config) {
  auto maybe_bus = I2CBus::open(config.i2c_bus);
  if (not maybe_bus) {
    panic<Exception>(std::format("{}", maybe_bus.error().message()));
  }

  impl_ = std::make_unique<Impl>(std::move(*maybe_bus));

  // Verify device identity
  auto who_am_i = std::uint8_t{};
  if (const auto rc =
          impl_->bus.read(lps25h::ADDR, lps25h::WHO_AM_I, std::span<std::uint8_t>(&who_am_i, 1));
      not rc) {
    panic<Exception>(std::format("ERROR reading WHO_AM_I: {}", rc.error().message()));
  }
  if (who_am_i != lps25h::WHO_AM_I_VAL) {
    panic<Exception>(std::format("Unexpected WHO_AM_I 0x{:02X} (expected 0x{:02X})", who_am_i,
                                 lps25h::WHO_AM_I_VAL));
  }

  if (const auto rc = impl_->bus.write(lps25h::ADDR, lps25h::CTRL_REG1,
                                       std::array<std::uint8_t, 1>{ lps25h::CFG_25HZ });
      not rc) {
    panic<Exception>(std::format("ERROR powering up: {}", rc.error().message()));
  }
}

//-------------------------------------------------------------------------------------------------
auto PressureSensor::read() const -> std::expected<Measurement, Error> {
  // Poll STATUS_REG until both P_DA and T_DA are set
  static constexpr auto POLL_TIMEOUT = std::chrono::milliseconds(100);
  static constexpr auto POLL_INTERVAL = std::chrono::milliseconds(5);
  const auto deadline = std::chrono::steady_clock::now() + POLL_TIMEOUT;

  auto status = std::uint8_t{};
  while (true) {
    if (const auto rc =
            impl_->bus.read(lps25h::ADDR, lps25h::STATUS_REG, std::span<std::uint8_t>(&status, 1));
        not rc) {
      return std::unexpected(rc.error());
    }
    static constexpr auto HAS_DATA_MASK = static_cast<std::uint32_t>(lps25h::P_DA | lps25h::T_DA);
    if ((static_cast<std::uint32_t>(status) & HAS_DATA_MASK) == HAS_DATA_MASK) {
      break;
    }
    if (std::chrono::steady_clock::now() > deadline) {
      return std::unexpected(Error{ "Timed out waiting for data" });
    }
    std::this_thread::sleep_for(POLL_INTERVAL);
  }

  // Burst read 3 pressure bytes: PRESS_OUT_XL, PRESS_OUT_L, PRESS_OUT_H
  auto press_buf = std::array<std::uint8_t, 3>{};
  if (const auto rc =
          impl_->bus.read(lps25h::ADDR, lps25h::PRESS_OUT_XL | lps25h::MULTI_BYTE, press_buf);
      not rc) {
    return std::unexpected(rc.error());
  }

  // Burst read 2 temperature bytes: TEMP_OUT_L, TEMP_OUT_H
  auto temp_buf = std::array<std::uint8_t, 2>{};
  if (const auto rc =
          impl_->bus.read(lps25h::ADDR, lps25h::TEMP_OUT_L | lps25h::MULTI_BYTE, temp_buf);
      not rc) {
    return std::unexpected(rc.error());
  }

  // Decode: pressure is 24-bit unsigned, temperature is 16-bit signed
  // Reference: ST Technical Note TN1228
  const auto pressure_raw =
      static_cast<std::int32_t>(toUint32(0, press_buf[2], press_buf[1], press_buf[0]));
  const auto temp_raw = static_cast<std::int16_t>(toUint16(temp_buf[1], temp_buf[0]));

  return Measurement{ .timestamp = std::chrono::system_clock::now(),
                      .pressure_hpa = static_cast<float>(pressure_raw) * lps25h::PRESSURE_SCALE,
                      .temperature_celsius = lps25h::TEMP_OFFSET +
                                             (static_cast<float>(temp_raw) * lps25h::TEMP_SCALE) };
}

}  // namespace grape::rpi::sense_hat
