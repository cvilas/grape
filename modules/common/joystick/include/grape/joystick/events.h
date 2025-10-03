//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <functional>
#include <variant>

#include "grape/joystick/controls.h"
#include "grape/wall_clock.h"

namespace grape::joystick {

struct ButtonEvent {
  WallClock::TimePoint timestamp;
  ControlId id{};
  bool pressed{ false };
};

struct AxisEvent {
  WallClock::TimePoint timestamp;
  ControlId id{};
  float value{ 0.F };  //!< Normalised value in range [-1, 1]
};

struct ConnectionEvent {
  WallClock::TimePoint timestamp;
  bool is_connected{ false };
};

struct ErrorEvent {
  WallClock::TimePoint timestamp;
  std::string message;
};

using Event = std::variant<ButtonEvent, AxisEvent, ErrorEvent, ConnectionEvent>;
using EventCallback = std::function<void(const Event&)>;

}  // namespace grape::joystick
