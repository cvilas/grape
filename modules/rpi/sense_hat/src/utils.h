//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>

namespace grape::rpi::sense_hat {

constexpr auto toUint16(std::uint8_t high, std::uint8_t low) -> std::uint16_t {
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  return static_cast<std::uint16_t>(high << 8U) | static_cast<std::uint16_t>(low);
}

constexpr auto toUint32(std::uint8_t b3, std::uint8_t b2, std::uint8_t b1, std::uint8_t b0)
    -> std::uint32_t {
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
  return (static_cast<std::uint32_t>(b3) << 24U) | (static_cast<std::uint32_t>(b2) << 16U) |
         (static_cast<std::uint32_t>(b1) << 8U) | static_cast<std::uint32_t>(b0);
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
}

}  // namespace grape::rpi::sense_hat
