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
  std::uint64_t nanoseconds{};
  Position position{};
  Quaternion orientation{};
};

//-------------------------------------------------------------------------------------------------
template <grape::serdes::WritableStream OutStream>
[[nodiscard]] auto pack(grape::serdes::Serialiser<OutStream>& ser, const Position& pos) -> bool {
  if (not ser.pack(pos.x)) {
    return false;
  }
  if (not ser.pack(pos.y)) {
    return false;
  }
  if (not ser.pack(pos.z)) {
    return false;
  }
  return true;
}

//-------------------------------------------------------------------------------------------------
template <grape::serdes::ReadableStream InStream>
[[nodiscard]] auto unpack(grape::serdes::Deserialiser<InStream>& des, Position& pos) -> bool {
  if (not des.unpack(pos.x)) {
    return false;
  }
  if (not des.unpack(pos.y)) {
    return false;
  }
  if (not des.unpack(pos.z)) {
    return false;
  }
  return true;
}

//-------------------------------------------------------------------------------------------------
template <grape::serdes::WritableStream OutStream>
[[nodiscard]] auto pack(grape::serdes::Serialiser<OutStream>& ser, const Quaternion& qt) -> bool {
  if (not ser.pack(qt.x)) {
    return false;
  }
  if (not ser.pack(qt.y)) {
    return false;
  }
  if (not ser.pack(qt.z)) {
    return false;
  }
  if (not ser.pack(qt.w)) {
    return false;
  }
  return true;
}

//-------------------------------------------------------------------------------------------------
template <grape::serdes::ReadableStream InStream>
[[nodiscard]] auto unpack(grape::serdes::Deserialiser<InStream>& des, Quaternion& qt) -> bool {
  if (not des.unpack(qt.x)) {
    return false;
  }
  if (not des.unpack(qt.y)) {
    return false;
  }
  if (not des.unpack(qt.z)) {
    return false;
  }
  if (not des.unpack(qt.w)) {
    return false;
  }
  return true;
}

//-------------------------------------------------------------------------------------------------
template <grape::serdes::WritableStream OutStream>
[[nodiscard]] auto pack(grape::serdes::Serialiser<OutStream>& ser, const PoseStamped& pos) -> bool {
  if (not ser.pack(pos.nanoseconds)) {
    return false;
  }
  if (not pack(ser, pos.position)) {
    return false;
  }
  if (not pack(ser, pos.orientation)) {
    return false;
  }
  return true;
}

//-------------------------------------------------------------------------------------------------
template <grape::serdes::ReadableStream InStream>
[[nodiscard]] auto unpack(grape::serdes::Deserialiser<InStream>& des, PoseStamped& pos) -> bool {
  if (not des.unpack(pos.nanoseconds)) {
    return false;
  }
  if (not unpack(des, pos.position)) {
    return false;
  }
  if (not unpack(des, pos.orientation)) {
    return false;
  }
  return true;
}
