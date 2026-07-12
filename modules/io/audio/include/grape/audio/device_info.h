//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace grape::audio {

/// I/O direction
enum class Direction : std::uint8_t {
  Input,
  Output,
};

/// Device information
struct DeviceInfo {
  std::uint32_t id;
  std::string name;
};

/// Enumerate devices
/// @param dir Direction of data flow
/// @return List of devices
[[nodiscard]] auto enumerate(Direction dir) -> std::vector<DeviceInfo>;

}  // namespace grape::audio
