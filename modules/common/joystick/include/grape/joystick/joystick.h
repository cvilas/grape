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

///@return List of connected joystick devices
[[nodiscard]] auto enumerate() -> std::vector<std::filesystem::path>;

/// @return Reads hardware information for a specified joystick device, or error message on failure
[[nodiscard]] auto readDeviceInfo(const std::filesystem::path& path)
    -> std::expected<DeviceInfo, std::string>;

//=================================================================================================
/// Joystick interface
///
/// Note that methods of this class are not thread-safe; use in a single-threaded context.
///
/// See examples for usage
class Joystick {
public:
  /// Constructor
  /// @param cb Event callback to trigger on activity
  explicit Joystick(EventCallback&& cb);

  /// Open a specified joystick device
  /// @param device_path Path to joystick device as returned by enumerate()
  /// @return true on success, false on failure (error callback is triggered)
  [[nodiscard]] auto open(const std::filesystem::path& device_path) -> bool;

  /// Close the currently opened joystick device
  void close();

  /// Wait for joystick events. Callback is automatically triggered on each event.
  /// @return true on nominal operation. False on error; call open() again to reinitialise
  [[nodiscard]] auto wait() -> bool;

  /// Wait for joystick events with a timeout.
  /// @param timeout Maximum time to wait for an event. If negative, waits indefinitely.
  /// @return true on nominal operation. False on error; call open() again to reinitialise
  [[nodiscard]] auto wait(std::chrono::milliseconds timeout) -> bool;

  ~Joystick();
  Joystick(const Joystick&) = delete;
  Joystick(Joystick&&) = delete;
  auto operator=(const Joystick&) = delete;
  auto operator=(Joystick&&) = delete;

private:
  [[nodiscard]] auto readState() -> bool;
  [[nodiscard]] auto normalise(ControlId axis, std::int32_t value) const -> float;
  struct Impl;
  std::unique_ptr<Impl> impl_{ nullptr };
};

}  // namespace grape::joystick
