//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>
#include <span>

#include <SDL3/SDL_pixels.h>

#include "grape/serdes/concepts.h"
#include "grape/serdes/serdes.h"

struct CameraFrameInfo {
  SDL_PixelFormat format;        // SDL specific pixel format
  std::int32_t width;            // Image width in pixels
  std::int32_t height;           // Image height in pixels
  std::int32_t pitch_bytes;      // distance in bytes between rows of pixels
  std::uint64_t timestamp_ns;    // Capture timestamp in nanoseconds (since arbitrary epoch)
  std::span<const char> pixels;  // pixel data
};

//-------------------------------------------------------------------------------------------------
template <grape::serdes::WritableStream OutStream>
[[nodiscard]] auto pack(grape::serdes::Serialiser<OutStream>& ser, const CameraFrameInfo& info)
    -> bool {
  if (not ser.pack(info.format)) {
    return false;
  }
  if (not ser.pack(info.width)) {
    return false;
  }
  if (not ser.pack(info.height)) {
    return false;
  }
  if (not ser.pack(info.pitch_bytes)) {
    return false;
  }
  if (not ser.pack(info.timestamp_ns)) {
    return false;
  }
  if (not ser.pack(info.pixels)) {
    return false;
  }
  return true;
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] auto deserialize(Deserialiser& des, State& st) -> bool {
  if (not des.unpack(st.name)) {
    return false;
  }
  if (not des.unpack(st.timestamp)) {
    return false;
  }
  if (not des.unpack(st.position)) {
    return false;
  }
  return true;
}
