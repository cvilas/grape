//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/rpi/sense_hat/inertial_sensor.h"

#include <utility>

#include "grape/exception.h"
#include "lsm9ds1.h"
#include "utils.h"

namespace grape::rpi::sense_hat {

namespace {

//-------------------------------------------------------------------------------------------------
// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
auto parseVec3(std::span<const std::uint8_t, 6> buf) -> Vec3<std::int16_t> {
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
  return { .x = static_cast<std::int16_t>(toUint16(buf[1], buf[0])),
           .y = static_cast<std::int16_t>(toUint16(buf[3], buf[2])),
           .z = static_cast<std::int16_t>(toUint16(buf[5], buf[4])) };
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
}
}  // namespace

//=================================================================================================
struct InertialSensor::Impl {
  explicit Impl(I2CBus i2c_bus) : bus(std::move(i2c_bus)) {
  }
  I2CBus bus;
  std::uint64_t tick{ 0 };
  Vec3<std::int16_t> last_mag{};  //!< Most recent mag reading (persisted across non-mag ticks)
};

//-------------------------------------------------------------------------------------------------
InertialSensor::~InertialSensor() {
  if (impl_ != nullptr) {
    std::ignore = impl_->bus.write(lsm9ds1::AG_ADDR, lsm9ds1::ag::CTRL_REG1_G,
                                   std::array<std::uint8_t, 1>{ lsm9ds1::ag::ODR_POWER_DOWN });
    std::ignore = impl_->bus.write(lsm9ds1::AG_ADDR, lsm9ds1::ag::CTRL_REG6_XL,
                                   std::array<std::uint8_t, 1>{ lsm9ds1::ag::ODR_POWER_DOWN });
    std::ignore = impl_->bus.write(lsm9ds1::MAG_ADDR, lsm9ds1::mag::CTRL_REG3_M,
                                   std::array<std::uint8_t, 1>{ lsm9ds1::mag::MODE_POWER_DOWN });
  }
}

//-------------------------------------------------------------------------------------------------
InertialSensor::InertialSensor(const Config& config) {
  auto maybe_bus = I2CBus::open(config.i2c_bus);
  if (not maybe_bus) {
    panic<Exception>(std::format("{}", maybe_bus.error().message()));
  }
  impl_ = std::make_unique<Impl>(std::move(*maybe_bus));

  // Verify accel/gyro identity
  auto ag_id = std::uint8_t{};
  if (const auto rc = impl_->bus.read(lsm9ds1::AG_ADDR, lsm9ds1::ag::WHO_AM_I,
                                      std::span<std::uint8_t>(&ag_id, 1));
      not rc) {
    panic<Exception>(std::format("ERROR reading AG WHO_AM_I: {}", rc.error().message()));
  }
  if (ag_id != lsm9ds1::AG_WHO_AM_I) {
    panic<Exception>(std::format("Unexpected AG WHO_AM_I 0x{:02X} (expected 0x{:02X})", ag_id,
                                 lsm9ds1::AG_WHO_AM_I));
  }

  // Verify magnetometer identity
  auto mag_id = std::uint8_t{};
  if (const auto rc = impl_->bus.read(lsm9ds1::MAG_ADDR, lsm9ds1::mag::WHO_AM_I,
                                      std::span<std::uint8_t>(&mag_id, 1));
      not rc) {
    panic<Exception>(std::format("ERROR reading MAG WHO_AM_I: {}", rc.error().message()));
  }
  if (mag_id != lsm9ds1::MAG_WHO_AM_I) {
    panic<Exception>(std::format("Unexpected MAG WHO_AM_I 0x{:02X} (expected 0x{:02X})", mag_id,
                                 lsm9ds1::MAG_WHO_AM_I));
  }

  // Enable auto-increment for burst reads on AG
  if (const auto rc = impl_->bus.write(lsm9ds1::AG_ADDR, lsm9ds1::ag::CTRL_REG8,
                                       std::array<std::uint8_t, 1>{ lsm9ds1::ag::IF_ADD_INC });
      not rc) {
    panic<Exception>(std::format("ERROR writing CTRL_REG8: {}", rc.error().message()));
  }

  // Configure gyroscope ODR and full-scale (default 245 dps)
  if (const auto rc = impl_->bus.write(lsm9ds1::AG_ADDR, lsm9ds1::ag::CTRL_REG1_G,
                                       std::array<std::uint8_t, 1>{ lsm9ds1::ag::ODR_476HZ });
      not rc) {
    panic<Exception>(std::format("ERROR writing CTRL_REG1_G: {}", rc.error().message()));
  }

  // Configure accelerometer ODR and full-scale (default +/-2 g)
  if (const auto rc = impl_->bus.write(lsm9ds1::AG_ADDR, lsm9ds1::ag::CTRL_REG6_XL,
                                       std::array<std::uint8_t, 1>{ lsm9ds1::ag::ODR_476HZ });
      not rc) {
    panic<Exception>(std::format("ERROR writing CTRL_REG6_XL: {}", rc.error().message()));
  }

  // Configure magnetometer: 80 Hz, ultra-high performance, continuous
  if (const auto rc = impl_->bus.write(lsm9ds1::MAG_ADDR, lsm9ds1::mag::CTRL_REG1_M,
                                       std::array<std::uint8_t, 1>{ lsm9ds1::mag::CFG_80HZ_ULTRA });
      not rc) {
    panic<Exception>(std::format("ERROR writing CTRL_REG1_M: {}", rc.error().message()));
  }
  if (const auto rc = impl_->bus.write(lsm9ds1::MAG_ADDR, lsm9ds1::mag::CTRL_REG2_M,
                                       std::array<std::uint8_t, 1>{ 0x00U });  // +/-4 gauss
      not rc) {
    panic<Exception>(std::format("ERROR writing CTRL_REG2_M: {}", rc.error().message()));
  }
  if (const auto rc =
          impl_->bus.write(lsm9ds1::MAG_ADDR, lsm9ds1::mag::CTRL_REG3_M,
                           std::array<std::uint8_t, 1>{ lsm9ds1::mag::MODE_CONTINUOUS });
      not rc) {
    panic<Exception>(std::format("ERROR writing CTRL_REG3_M: {}", rc.error().message()));
  }
  if (const auto rc = impl_->bus.write(lsm9ds1::MAG_ADDR, lsm9ds1::mag::CTRL_REG4_M,
                                       std::array<std::uint8_t, 1>{ lsm9ds1::mag::Z_ULTRA_HIGH });
      not rc) {
    panic<Exception>(std::format("ERROR writing CTRL_REG4_M: {}", rc.error().message()));
  }

  impl_->tick = 0;
  impl_->last_mag = {};
}

//-------------------------------------------------------------------------------------------------
auto InertialSensor::read() -> std::expected<ImuSample, Error> {
  ImuSample sample{};
  sample.timestamp = WallClock::now();

  auto buf = std::array<std::uint8_t, 6>{};  // NOLINT(cppcoreguidelines-avoid-magic-numbers)

  if (const auto rc = impl_->bus.read(lsm9ds1::AG_ADDR, lsm9ds1::ag::OUT_X_L_G, buf); not rc) {
    return std::unexpected(rc.error());
  }
  sample.gyro = parseVec3(buf);

  if (const auto rc = impl_->bus.read(lsm9ds1::AG_ADDR, lsm9ds1::ag::OUT_X_L_XL, buf); not rc) {
    return std::unexpected(rc.error());
  }
  sample.accel = parseVec3(buf);

  // Read mag every Nth tick
  static constexpr auto MAG_READ_DIVISOR = 6;
  if ((impl_->tick % MAG_READ_DIVISOR) == 0) {
    // Mag subsystem uses the STMicro 0x80 address-bit convention for auto-increment
    // (unlike AG, which uses the IF_ADD_INC bit in CTRL_REG8)
    if (const auto rc =
            impl_->bus.read(lsm9ds1::MAG_ADDR, lsm9ds1::mag::OUT_X_L_M | lsm9ds1::AUTO_INC, buf);
        not rc) {
      return std::unexpected(rc.error());
    }
    impl_->last_mag = parseVec3(buf);
    sample.mag_updated = true;
  }
  sample.mag = impl_->last_mag;
  ++impl_->tick;
  return sample;
}

}  // namespace grape::rpi::sense_hat
