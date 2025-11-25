//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>
#include <filesystem>
#include <span>

#include "grape/wall_clock.h"

namespace grape::camera {

//=================================================================================================
/// Single image frame data
struct ImageFrame {
  struct Header {
    WallClock::TimePoint timestamp;  //!< Acquistion timestamp
    std::uint32_t pitch{};           //!< Bytes per row of pixels
    std::uint16_t width{};           //!< Width of the image in pixels
    std::uint16_t height{};          //!< Height of the image in pixels (number of rows)
    std::uint32_t format{};          //!< driver backend-specific pixel format
  };
  Header header;
  std::span<std::byte> pixels;  //!< pixel data
};

/// @return true if image dimensions and format matches
constexpr auto matchesFormat(const ImageFrame::Header& hd1, const ImageFrame::Header& hd2) -> bool {
  return std::tie(hd1.width, hd1.height, hd1.format) == std::tie(hd2.width, hd2.height, hd2.format);
}

/// Save an image frame to a file
/// @param frame The image frame to save
/// @param fname The file path to save the image frame to
/// @return true if the image frame was saved successfully, false otherwise
[[nodiscard]] auto save(const ImageFrame& frame, const std::filesystem::path& fname) -> bool;

}  // namespace grape::camera
