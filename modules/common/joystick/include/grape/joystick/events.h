//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <chrono>
#include <variant>

#include "grape/joystick/controls.h"

namespace grape::joystick {

using Clock = std::chrono::system_clock;

struct ControlEvent {
  Clock::time_point timestamp;
  std::int32_t value{};
  ControlType type{};
  ControlId id{};
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

}  // namespace grape::joystick
