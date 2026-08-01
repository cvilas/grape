//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/utils/enums.h"

namespace grape::joystick {

// Types of supported controls on a joystick
enum class ControlType : std::uint8_t {
  Unknown,
  Axis,    //!< Axis control
  Button,  //!< Button control
};

constexpr auto toString(ControlType type) -> std::string_view {
  return grape::enums::name(type);
}

// Identifiers for axes, keys and buttons
enum class ControlId : std::uint8_t {
  Unknown,
  AxisX,
  AxisY,
  AxisZ,
  AxisRx,
  AxisRy,
  AxisRz,
  AxisThrottle,
  AxisRudder,
  AxisWheel,
  AxisGas,
  AxisBrake,
  AxisHat0X,
  AxisHat0Y,
  AxisHat1X,
  AxisHat1Y,
  AxisHat2X,
  AxisHat2Y,
  AxisHat3X,
  AxisHat3Y,

  KeyEnter,
  KeyUp,
  KeyLeft,
  KeyRight,
  KeyDown,
  KeyBack,

  Button0,
  Button1,
  Button2,
  Button3,
  Button4,
  Button5,
  Button6,
  Button7,
  Button8,
  Button9,
  ButtonDpadUp,
  ButtonDpadDown,
  ButtonDpadLeft,
  ButtonDpadRight,
  ButtonLeft,
  ButtonRight,
  ButtonMiddle,
  ButtonSide,
  ButtonExtra,
  ButtonForward,
  ButtonBack,
  ButtonTask,
  ButtonTrigger,
  ButtonTriggerHappy1,
  ButtonTriggerHappy2,
  ButtonTriggerHappy3,
  ButtonTriggerHappy4,
  ButtonTriggerHappy5,
  ButtonTriggerHappy6,
  ButtonThumb,
  ButtonThumb2,
  ButtonTop,
  ButtonTop2,
  ButtonPinkie,
  ButtonBase,
  ButtonBase2,
  ButtonBase3,
  ButtonBase4,
  ButtonBase5,
  ButtonBase6,
  ButtonBase7,
  ButtonBase8,
  ButtonBase9,
  ButtonDead,
  ButtonA,
  ButtonB,
  ButtonC,
  ButtonX,
  ButtonY,
  ButtonZ,
  ButtonTL,
  ButtonTR,
  ButtonTL2,
  ButtonTR2,
  ButtonSelect,
  ButtonStart,
  ButtonMode,
  ButtonThumbL,
  ButtonThumbR,
  ButtonWheel,
  ButtonGearUp,
};

}  // namespace grape::joystick

template <>
struct grape::enums::Range<grape::joystick::ControlId> {
  static constexpr int MIN = static_cast<int>(grape::joystick::ControlId::Unknown);
  static constexpr int MAX = static_cast<int>(grape::joystick::ControlId::ButtonGearUp);
};

namespace grape::joystick {
constexpr auto toString(ControlId id) -> std::string_view {
  return grape::enums::name(id);
}
}  // namespace grape::joystick
