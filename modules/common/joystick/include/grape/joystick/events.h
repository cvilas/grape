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

struct ButtonEvent {
  Clock::time_point timestamp;
  ControlId id{};
  bool pressed{ false };
};

struct AxisEvent {
  Clock::time_point timestamp;
  ControlId id{};
  float value{ 0.F };  //!< Normalised value in range [-1, 1]
};

struct ConnectionEvent {
  Clock::time_point timestamp;
  bool is_connected{ false };
};

struct ErrorEvent {
  Clock::time_point timestamp;
  std::string message;
};

using Event = std::variant<ButtonEvent, AxisEvent, ErrorEvent, ConnectionEvent>;
using EventCallback = std::function<void(const Event&)>;

}  // namespace grape::joystick
