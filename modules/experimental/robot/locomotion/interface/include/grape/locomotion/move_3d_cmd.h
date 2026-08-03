//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <format>
#include <string>

namespace grape::locomotion {

//=================================================================================================
/// Command to move the robot platform in 3D space (planar or piecewise planar surfaces).
struct Move3DCmd {
  float forward_speed{ 0 };
  float lateral_speed{ 0 };
  float turn_speed{ 0 };
};

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toString(const Move3DCmd& cmd) -> std::string {
  return std::format("Move3DCmd{{forward_speed={}, lateral_speed={}, turn_speed={}}}",
                     cmd.forward_speed, cmd.lateral_speed, cmd.turn_speed);
}

}  // namespace grape::locomotion
