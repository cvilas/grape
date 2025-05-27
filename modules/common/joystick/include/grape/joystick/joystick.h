//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <expected>
#include <filesystem>
#include <memory>
#include <vector>

#include "grape/joystick/device_info.h"
#include "grape/joystick/events.h"

namespace grape::joystick {

[[nodiscard]] auto enumerate() -> std::vector<std::filesystem::path>;

[[nodiscard]] auto readDeviceInfo(const std::filesystem::path& path)
    -> std::expected<DeviceInfo, std::string>;

class Joystick {
public:
  using Capabilities = std::unordered_map<ControlType, std::vector<ControlId>>;

  explicit Joystick(EventCallback&& cb);

  auto open(const std::filesystem::path& device_path) -> bool;
  void close();
  auto wait() -> bool;
  auto wait(std::chrono::milliseconds timeout) -> bool;

  ~Joystick();
  Joystick(const Joystick&) = delete;
  Joystick(Joystick&&) = delete;
  auto operator=(const Joystick&) = delete;
  auto operator=(Joystick&&) = delete;

private:
  auto readState() -> bool;
  auto readCapabilities() -> std::expected<Capabilities, std::string>;
  struct Impl;
  std::unique_ptr<Impl> impl_{ nullptr };
};

//[[nodiscard]] auto range(ControlId axis) const -> Range;
// void setRange(ControlId axis, const Range& cfg);

}  // namespace grape::joystick
