//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <string>

#include "grape/serdes/serdes.h"

namespace grape::robot::loco {

//=================================================================================================
/// A no-op locomotion command to keep connection stream alive
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
}  // namespace grape::robot::loco
