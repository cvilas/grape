//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/rpi/sense_hat/humidity_sensor.h"

#include <array>
#include <format>
#include <thread>

#include "grape/exception.h"
#include "hts221.h"
#include "utils.h"

namespace grape::rpi::sense_hat {

//=================================================================================================
struct HumiditySensor::Impl {
  struct Calibration {
    float h0_rh{};
    float h1_rh{};
    float t0_degc{};
    float t1_degc{};
    std::int16_t h0_t0_out{};
    std::int16_t h1_t0_out{};
    std::int16_t t0_out{};
    std::int16_t t1_out{};
  };

  explicit Impl(I2CBus i2c_bus) : bus(std::move(i2c_bus)) {
  }
  I2CBus bus;
  Calibration cal{};
};

//-------------------------------------------------------------------------------------------------
HumiditySensor::~HumiditySensor() {
  if (impl_ != nullptr) {
    std::ignore = impl_->bus.write(hts221::ADDR, hts221::CTRL_REG1,
                                   std::array<std::uint8_t, 1>{ hts221::POWER_DOWN });
  }
}

//-------------------------------------------------------------------------------------------------
HumiditySensor::HumiditySensor(const Config& config) {
  auto maybe_bus = I2CBus::open(config.i2c_bus);
  if (not maybe_bus) {
    panic<Exception>(std::format("{}", maybe_bus.error().message()));
  }

  impl_ = std::make_unique<Impl>(std::move(*maybe_bus));

  // Verify device identity
  auto who_am_i = std::uint8_t{};
  if (const auto rc =
          impl_->bus.read(hts221::ADDR, hts221::WHO_AM_I, std::span<std::uint8_t>(&who_am_i, 1));
      not rc) {
    panic<Exception>(std::format("ERROR reading WHO_AM_I: {}", rc.error().message()));
  }
  if (who_am_i != hts221::WHO_AM_I_VAL) {
    panic<Exception>(std::format("Unexpected WHO_AM_I 0x{:02X} (expected 0x{:02X})", who_am_i,
                                 hts221::WHO_AM_I_VAL));
  }

  // Read calibration registers (16-byte burst from 0x30)
  auto calib = std::array<std::uint8_t, hts221::CALIB_LEN>{};
  if (const auto rc =
          impl_->bus.read(hts221::ADDR, hts221::CALIB_START | hts221::MULTI_BYTE, calib);
      not rc) {
    panic<Exception>(std::format("ERROR reading calibration: {}", rc.error().message()));
  }

  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

  // Parse calibration
  impl_->cal.h0_rh = static_cast<float>(calib[0]) / 2.F;
  impl_->cal.h1_rh = static_cast<float>(calib[1]) / 2.F;

  // T0/T1 degC: 10-bit values split across calib[2]/calib[3] (low 8 bits) and calib[5] (MSBs)
  const auto t0_degc_x8 = toUint16(calib[5] & 0x03U, calib[2]);
  const auto t1_degc_x8 = toUint16(static_cast<std::uint8_t>(calib[5] >> 2U) & 0x03U, calib[3]);
  impl_->cal.t0_degc = static_cast<float>(t0_degc_x8) / 8.F;
  impl_->cal.t1_degc = static_cast<float>(t1_degc_x8) / 8.F;

  // ADC output at calibration reference points (signed 16-bit)
  impl_->cal.h0_t0_out = static_cast<std::int16_t>(toUint16(calib[7], calib[6]));
  impl_->cal.h1_t0_out = static_cast<std::int16_t>(toUint16(calib[11], calib[10]));
  impl_->cal.t0_out = static_cast<std::int16_t>(toUint16(calib[13], calib[12]));
  impl_->cal.t1_out = static_cast<std::int16_t>(toUint16(calib[15], calib[14]));

  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

  // Power on at 12.5 Hz continuous, block data update enabled
  if (const auto rc = impl_->bus.write(hts221::ADDR, hts221::CTRL_REG1,
                                       std::array<std::uint8_t, 1>{ hts221::CFG_12HZ });
      not rc) {
    panic<Exception>(std::format("ERROR powering up: {}", rc.error().message()));
  }
}

//-------------------------------------------------------------------------------------------------
auto HumiditySensor::read() const -> std::expected<Measurement, Error> {
  // Poll STATUS_REG until both H_DA and T_DA are set
  static constexpr auto POLL_TIMEOUT = std::chrono::milliseconds(200);
  static constexpr auto POLL_INTERVAL = std::chrono::milliseconds(10);
  const auto deadline = std::chrono::steady_clock::now() + POLL_TIMEOUT;

  auto status = std::uint8_t{};
  while (true) {
    if (const auto rc =
            impl_->bus.read(hts221::ADDR, hts221::STATUS_REG, std::span<std::uint8_t>(&status, 1));
        not rc) {
      return std::unexpected(rc.error());
    }
    static constexpr auto HAS_DATA_MASK = static_cast<std::uint32_t>(hts221::H_DA | hts221::T_DA);
    if ((static_cast<std::uint32_t>(status) & HAS_DATA_MASK) == HAS_DATA_MASK) {
      break;
    }
    if (std::chrono::steady_clock::now() > deadline) {
      return std::unexpected(Error{ "Timed out waiting for data" });
    }
    std::this_thread::sleep_for(POLL_INTERVAL);
  }

  // Burst read 2 humidity bytes: HUM_OUT_L, HUM_OUT_H
  auto hum_buf = std::array<std::uint8_t, 2>{};
  if (const auto rc =
          impl_->bus.read(hts221::ADDR, hts221::HUM_OUT_L | hts221::MULTI_BYTE, hum_buf);
      not rc) {
    return std::unexpected(rc.error());
  }

  // Burst read 2 temperature bytes: TEMP_OUT_L, TEMP_OUT_H
  auto temp_buf = std::array<std::uint8_t, 2>{};
  if (const auto rc =
          impl_->bus.read(hts221::ADDR, hts221::TEMP_OUT_L | hts221::MULTI_BYTE, temp_buf);
      not rc) {
    return std::unexpected(rc.error());
  }

  const auto hum_raw = static_cast<std::int16_t>(toUint16(hum_buf[1], hum_buf[0]));
  const auto temp_raw = static_cast<std::int16_t>(toUint16(temp_buf[1], temp_buf[0]));

  // Linear interpolation using calibration reference points
  // Reference: ST Technical Note TN1218
  const auto& cal = impl_->cal;
  const float humidity =
      cal.h0_rh + ((cal.h1_rh - cal.h0_rh) * static_cast<float>(hum_raw - cal.h0_t0_out) /
                   static_cast<float>(cal.h1_t0_out - cal.h0_t0_out));
  const float temperature =
      cal.t0_degc + ((cal.t1_degc - cal.t0_degc) * static_cast<float>(temp_raw - cal.t0_out) /
                     static_cast<float>(cal.t1_out - cal.t0_out));

  return Measurement{ .timestamp = std::chrono::system_clock::now(),
                      .relative_humidity_percent = humidity,
                      .temperature_celsius = temperature };
}

}  // namespace grape::rpi::sense_hat
