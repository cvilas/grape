//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>
#include <string>
#include <variant>

#include "grape/serdes/serdes.h"

namespace grape::robot::loco {

//=================================================================================================
struct KeepAliveCmd {
  bool alive{ true };
};

//=================================================================================================
struct Move3DCmd {
  float forward_speed{ 0 };
  float lateral_speed{ 0 };
  float turn_speed{ 0 };
};

using Command = std::variant<KeepAliveCmd, Move3DCmd>;

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toString(const KeepAliveCmd& cmd) -> std::string {
  return "KeepAliveCmd{alive=" + std::string(cmd.alive ? "true" : "false") + "}";
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toString(const Move3DCmd& cmd) -> std::string {
  return "Move3DCmd{forward_speed=" + std::to_string(cmd.forward_speed) +
         ", lateral_speed=" + std::to_string(cmd.lateral_speed) +
         ", turn_speed=" + std::to_string(cmd.turn_speed) + "}";
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toString(const Command& cmd) -> std::string {
  return std::visit([](const auto& var) -> std::string { return toString(var); }, cmd);
}

//=================================================================================================
struct Status {
  std::uint64_t teleop_controller_id{ 0 };
};

//-------------------------------------------------------------------------------------------------
template <serdes::WritableStream S>
[[nodiscard]] auto serialise(serdes::Serialiser<S>& ser, const KeepAliveCmd& data) -> bool {
  return ser.pack(data.alive);
}

//-------------------------------------------------------------------------------------------------
template <serdes::ReadableStream S>
[[nodiscard]] auto deserialise(serdes::Deserialiser<S>& des, KeepAliveCmd& data) -> bool {
  return des.unpack(data.alive);
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

//-------------------------------------------------------------------------------------------------
template <serdes::WritableStream S>
[[nodiscard]] auto serialise(serdes::Serialiser<S>& ser, const Status& data) -> bool {
  return ser.pack(data.teleop_controller_id);
}

//-------------------------------------------------------------------------------------------------
template <serdes::ReadableStream S>
[[nodiscard]] auto deserialise(serdes::Deserialiser<S>& des, Status& data) -> bool {
  return des.unpack(data.teleop_controller_id);
}

}  // namespace grape::robot::loco
