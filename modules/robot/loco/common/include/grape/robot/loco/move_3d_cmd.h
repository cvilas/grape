//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <format>
#include <string>

#include "grape/serdes/serdes.h"

namespace grape::robot::loco {

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

//-------------------------------------------------------------------------------------------------
template <serdes::WritableStream S>
[[nodiscard]] auto serialise(serdes::Serialiser<S>& ser, const Move3DCmd& data) -> bool {
  return ser.pack(data.forward_speed) and ser.pack(data.lateral_speed) and
         ser.pack(data.turn_speed);
}

//-------------------------------------------------------------------------------------------------
template <serdes::ReadableStream S>
[[nodiscard]] auto deserialise(serdes::Deserialiser<S>& des, Move3DCmd& data) -> bool {
  return des.unpack(data.forward_speed) and des.unpack(data.lateral_speed) and
         des.unpack(data.turn_speed);
}

}  // namespace grape::robot::loco
