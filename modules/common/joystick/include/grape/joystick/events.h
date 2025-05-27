//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <chrono>
#include <functional>
#include <variant>

#include "grape/joystick/controls.h"

namespace grape::joystick {

using Clock = std::chrono::system_clock;

struct ControlEvent {
  Clock::time_point timestamp;
  ControlType type{};
  ControlId id{};
  std::int32_t value{};
};

struct ConnectionEvent {
  Clock::time_point timestamp;
  bool is_connected{ false };
};

struct ErrorEvent {
  Clock::time_point timestamp;
  std::string message;
};

using Event = std::variant<ControlEvent, ErrorEvent, ConnectionEvent>;
using EventCallback = std::function<void(const Event&)>;

}  // namespace grape::joystick
