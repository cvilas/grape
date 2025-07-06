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
  [[nodiscard]] auto open(const std::filesystem::path& device_path) const -> bool;

  /// Close the currently opened joystick device
  void close() const;

  /// Process all events accumulated since the last call to this method. Blocks until at least one
  /// event is available or the timeout expires. Triggers the registered callback for each event.
  /// @param timeout Maximum time to wait for an event. If zero, returns immediately. If negative,
  ///                waits indefinitely until an event arrives.
  /// @return true on successful operation (including timeout with no events).
  ///         false on device error, disconnection, or I/O failure.
  ///         When false is returned, the device should be considered unusable
  ///         and open() must be called again to reinitialize the connection.
  /// @note The callback function is invoked synchronously for each event during
  ///       this method's execution. Ensure the callback doesn't block for extended
  ///       periods to maintain responsive event processing.
  /// @see EventCallback for event handling interface
  [[nodiscard]] auto process(std::chrono::milliseconds timeout) const -> bool;

  ~Joystick();
  Joystick(const Joystick&) = delete;
  Joystick(Joystick&&) = delete;
  auto operator=(const Joystick&) = delete;
  auto operator=(Joystick&&) = delete;

private:
  [[nodiscard]] auto readState() const -> bool;
  struct Impl;
  std::unique_ptr<Impl> impl_{ nullptr };
};

}  // namespace grape::joystick
