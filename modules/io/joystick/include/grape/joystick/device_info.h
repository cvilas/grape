//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace grape::joystick {

/// Hardware information for a device
struct [[nodiscard]] DeviceInfo {
  std::string name;
  std::filesystem::path path;
  std::uint16_t bus_type{};
  std::uint16_t vendor_id{};
  std::uint16_t product_id{};
  std::uint16_t hw_version{};
};
}  // namespace grape::joystick
