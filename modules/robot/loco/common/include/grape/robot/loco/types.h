//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <chrono>
#include <cstdint>
#include <format>
#include <string>
#include <type_traits>
#include <variant>

#include "grape/serdes/serdes.h"

namespace grape::robot::loco {

//=================================================================================================
/// A no-op locomotion command to keep commection stream alive
///
/// Command prevents a connection from timing out when no other locomotion commands are sent.
/// Expected to perform no action on the robot.
struct KeepAliveCmd {};

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toString(const KeepAliveCmd& cmd) -> std::string {
  (void)cmd;
  return "KeepAliveCmd{}";
}

//-------------------------------------------------------------------------------------------------
template <serdes::WritableStream S>
[[nodiscard]] auto serialise(serdes::Serialiser<S>& ser, const KeepAliveCmd& data) -> bool {
  return ser.pack(sizeof(data));
}

//-------------------------------------------------------------------------------------------------
template <serdes::ReadableStream S>
[[nodiscard]] auto deserialise(serdes::Deserialiser<S>& des, KeepAliveCmd& data) -> bool {
  (void)data;
  auto sz = std::size_t{};
  return des.unpack(sz);
}

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

using Command = std::variant<KeepAliveCmd, Move3DCmd>;

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toString(const Command& cmd) -> std::string {
  return std::visit([](const auto& var) -> std::string { return toString(var); }, cmd);
}

//=================================================================================================
/// Status of the robot's locomotion service
struct Status {
  std::uint64_t teleop_controller_id{ 0 };
  std::chrono::system_clock::duration teleop_command_latency{ 0 };
};

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toString(const Status& cmd) -> std::string {
  return std::format("Status{{teleop_controller_id={:#x}, teleop_command_latency={}}}",
                     cmd.teleop_controller_id, cmd.teleop_command_latency.count());
}

//-------------------------------------------------------------------------------------------------
template <serdes::WritableStream S>
[[nodiscard]] auto serialise(serdes::Serialiser<S>& ser, const Status& data) -> bool {
  return ser.pack(data.teleop_controller_id) and ser.pack(data.teleop_command_latency.count());
}

//-------------------------------------------------------------------------------------------------
template <serdes::ReadableStream S>
[[nodiscard]] auto deserialise(serdes::Deserialiser<S>& des, Status& data) -> bool {
  if (not des.unpack(data.teleop_controller_id)) {
    return false;
  }
  std::chrono::system_clock::duration::rep ticks{};
  if (not des.unpack(ticks)) {
    return false;
  }
  data.teleop_command_latency = std::chrono::system_clock::duration(ticks);
  return true;
}

}  // namespace grape::robot::loco
