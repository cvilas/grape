//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <compare>
#include <cstdint>

namespace grape::camera {

/// Image dimensions in pixels
struct ImageSize {
  std::uint16_t width;
  std::uint16_t height;
  constexpr auto operator<=>(const ImageSize&) const = default;
};

/// Image capture specification
struct ImageSpec {
  ImageSize size;
  std::uint32_t pixel_format;
  constexpr auto operator<=>(const ImageSpec&) const = default;
};

/// Generates four character identifiers for pixel data packing format.
/// Roughly follows https://fourcc.org/, but don't count on consistency
constexpr auto fourcc(char ca, char cb, char cc, char cd) -> std::uint32_t {
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
  return (static_cast<std::uint32_t>(ca) << 0U) |   //
         (static_cast<std::uint32_t>(cb) << 8U) |   //
         (static_cast<std::uint32_t>(cc) << 16U) |  //
         (static_cast<std::uint32_t>(cd) << 24U);
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
}

// Supported fixel formats
constexpr auto PIXELFORMAT_NV12 = fourcc('N', 'V', '1', '2');
constexpr auto PIXELFORMAT_YV12 = fourcc('Y', 'V', '1', '2');
constexpr auto PIXELFORMAT_YUY2 = fourcc('Y', 'U', 'Y', '2');
constexpr auto PIXELFORMAT_MJPG = fourcc('M', 'J', 'P', 'G');
constexpr auto PIXELFORMAT_RGB24 = fourcc('R', 'G', '2', '4');

}  // namespace grape::camera
