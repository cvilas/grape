//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>

#include "grape/serdes/concepts.h"
#include "grape/serdes/serdes.h"

//-------------------------------------------------------------------------------------------------
struct Position {
  double x{};
  double y{};
  double z{};
};

//-------------------------------------------------------------------------------------------------
struct Quaternion {
  double x{};
  double y{};
  double z{};
  double w{};
};

//-------------------------------------------------------------------------------------------------
struct PoseStamped {
  std::int64_t nanoseconds{};
  Position position{};
  Quaternion orientation{};
};

//-------------------------------------------------------------------------------------------------
template <grape::serdes::WritableStream OutStream>
[[nodiscard]] auto serialise(grape::serdes::Serialiser<OutStream>& ser, const Position& pos)
    -> bool {
  return ser.pack(pos.x) and ser.pack(pos.y) and ser.pack(pos.z);
}

//-------------------------------------------------------------------------------------------------
template <grape::serdes::ReadableStream InStream>
[[nodiscard]] auto deserialise(grape::serdes::Deserialiser<InStream>& des, Position& pos) -> bool {
  return des.unpack(pos.x) and des.unpack(pos.y) and des.unpack(pos.z);
}

//-------------------------------------------------------------------------------------------------
template <grape::serdes::WritableStream OutStream>
[[nodiscard]] auto serialise(grape::serdes::Serialiser<OutStream>& ser, const Quaternion& qt)
    -> bool {
  return ser.pack(qt.x) and ser.pack(qt.y) and ser.pack(qt.z) and ser.pack(qt.w);
}

//-------------------------------------------------------------------------------------------------
template <grape::serdes::ReadableStream InStream>
[[nodiscard]] auto deserialise(grape::serdes::Deserialiser<InStream>& des, Quaternion& qt) -> bool {
  return des.unpack(qt.x) and des.unpack(qt.y) and des.unpack(qt.z) and des.unpack(qt.w);
}

//-------------------------------------------------------------------------------------------------
template <grape::serdes::WritableStream OutStream>
[[nodiscard]] auto serialise(grape::serdes::Serialiser<OutStream>& ser, const PoseStamped& pos)
    -> bool {
  return ser.pack(pos.nanoseconds) and ser.pack(pos.position) and ser.pack(pos.orientation);
}

//-------------------------------------------------------------------------------------------------
template <grape::serdes::ReadableStream InStream>
[[nodiscard]] auto deserialise(grape::serdes::Deserialiser<InStream>& des, PoseStamped& pos)
    -> bool {
  return des.unpack(pos.nanoseconds) and des.unpack(pos.position) and des.unpack(pos.orientation);
}
