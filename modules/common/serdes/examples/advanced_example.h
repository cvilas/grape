//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#pragma once

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
[[nodiscard]] auto pack(grape::serdes::Serialiser<OutStream>& ser, const Position& p) -> bool {
  if (not ser.pack(p.x)) {
    return false;
  }
  if (not ser.pack(p.y)) {
    return false;
  }
  if (not ser.pack(p.z)) {
    return false;
  }
  return true;
}

//-------------------------------------------------------------------------------------------------
template <grape::serdes::ReadableStream InStream>
[[nodiscard]] auto unpack(grape::serdes::Deserialiser<InStream>& des, Position& p) -> bool {
  if (not des.unpack(p.x)) {
    return false;
  }
  if (not des.unpack(p.y)) {
    return false;
  }
  if (not des.unpack(p.z)) {
    return false;
  }
  return true;
}

//-------------------------------------------------------------------------------------------------
template <grape::serdes::WritableStream OutStream>
[[nodiscard]] auto pack(grape::serdes::Serialiser<OutStream>& ser, const Quaternion& q) -> bool {
  if (not ser.pack(q.x)) {
    return false;
  }
  if (not ser.pack(q.y)) {
    return false;
  }
  if (not ser.pack(q.z)) {
    return false;
  }
  if (not ser.pack(q.w)) {
    return false;
  }
  return true;
}

//-------------------------------------------------------------------------------------------------
template <grape::serdes::ReadableStream InStream>
[[nodiscard]] auto unpack(grape::serdes::Deserialiser<InStream>& des, Quaternion& q) -> bool {
  if (not des.unpack(q.x)) {
    return false;
  }
  if (not des.unpack(q.y)) {
    return false;
  }
  if (not des.unpack(q.z)) {
    return false;
  }
  if (not des.unpack(q.w)) {
    return false;
  }
  return true;
}

//-------------------------------------------------------------------------------------------------
template <grape::serdes::WritableStream OutStream>
[[nodiscard]] auto pack(grape::serdes::Serialiser<OutStream>& ser, const PoseStamped& p) -> bool {
  if (not ser.pack(p.nanoseconds)) {
    return false;
  }
  if (not pack(ser, p.position)) {
    return false;
  }
  if (not pack(ser, p.orientation)) {
    return false;
  }
  return true;
}

//-------------------------------------------------------------------------------------------------
template <grape::serdes::ReadableStream InStream>
[[nodiscard]] auto unpack(grape::serdes::Deserialiser<InStream>& des, PoseStamped& p) -> bool {
  if (not des.unpack(p.nanoseconds)) {
    return false;
  }
  if (not unpack(des, p.position)) {
    return false;
  }
  if (not unpack(des, p.orientation)) {
    return false;
  }
  return true;
}
