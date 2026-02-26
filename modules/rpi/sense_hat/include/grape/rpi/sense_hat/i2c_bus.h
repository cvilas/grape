//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <span>

#include "grape/error.h"

namespace grape::rpi::sense_hat {

/// 7-bit I2C device address
enum class DevAddr : std::uint8_t {};

/// I2C register address
enum class RegAddr : std::uint8_t {};

/// Combine a register address with a modifier byte (e.g. auto-increment bit)
[[nodiscard]] constexpr auto operator|(RegAddr lhs, std::uint8_t rhs) -> RegAddr {
  return static_cast<RegAddr>(static_cast<std::uint8_t>(lhs) | rhs);
}

//=================================================================================================
/// Interface to devices on an i2c bus
///
class I2CBus {
public:
  /// Open an I2C bus
  /// @param bus Path to I2C bus (e.g., "/dev/i2c-1")
  /// @return Bus handle on success, error on failure
  [[nodiscard]] static auto open(const std::filesystem::path& bus) -> std::expected<I2CBus, Error>;

  /// Write one or more consecutive registers to a device on the bus.
  /// @param dev_addr 7-bit I2C device address
  /// @param reg Address of the first register to write
  /// @param data Bytes to write starting at @p reg. For a single byte, pass a 1-element span.
  /// @return Nothing on success, error on failure
  [[nodiscard]] auto write(DevAddr dev_addr, RegAddr reg, std::span<const std::uint8_t> data) const
      -> std::expected<void, Error>;

  /// Read one or more consecutive registers from a device on the bus.
  /// @param dev_addr 7-bit I2C device address
  /// @param reg Address of the first register to read
  /// @param data Buffer to receive bytes starting at @p reg. For a single byte, pass a 1-element
  /// span.
  /// @return Nothing on success, error on failure
  [[nodiscard]] auto read(DevAddr dev_addr, RegAddr reg, std::span<std::uint8_t> data) const
      -> std::expected<void, Error>;

  ~I2CBus();
  I2CBus(const I2CBus&) = delete;
  auto operator=(const I2CBus&) -> I2CBus& = delete;
  I2CBus(I2CBus&& other) noexcept;
  auto operator=(I2CBus&& other) noexcept -> I2CBus&;

private:
  void close();
  explicit I2CBus(int fd);
  int fd_{ -1 };
};

}  // namespace grape::rpi::sense_hat
