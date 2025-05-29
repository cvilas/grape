//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <filesystem>
#include <memory>
#include <stop_token>

#include "grape/joystick/events.h"

namespace grape::joystick {

struct [[nodiscard]] DeviceInfo {
  std::string name;
  std::filesystem::path path;
  std::uint16_t bus_type{};
  std::uint16_t vendor_id{};
  std::uint16_t product_id{};
  std::uint16_t hw_version{};
};

class Joystick {
public:
  struct [[nodiscard]] Range {
    std::int32_t minimum{};  //!< bottom end of the range
    std::int32_t maximum{};  //!< top end of the range
    std::int32_t fuzz{};  //!< noise band. Events may be ignored if value changes within this band
    std::int32_t flat{};  //!< deadband around zero (values discarded or reported as 0)
  };

  using EventCallback = std::function<void(const Event&)>;
  using Capabilities = std::unordered_map<ControlType, std::vector<ControlId>>;

  explicit Joystick(const std::filesystem::path& device_path, EventCallback&& cb);

  ~Joystick();
  Joystick(const Joystick&) = delete;
  Joystick(Joystick&&) = delete;
  auto operator=(const Joystick&) = delete;
  auto operator=(Joystick&&) = delete;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

[[nodiscard]] auto enumerate() -> std::vector<DeviceInfo>;

//[[nodiscard]] auto range(ControlId axis) const -> Range;
//[[nodiscard]] auto capabilities() const -> Capabilities;
// void setRange(ControlId axis, const Range& cfg);

}  // namespace grape::joystick
