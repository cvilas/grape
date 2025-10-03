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
    std::uint32_t pitch{};           //!< Bytes per row of pixels
    std::uint32_t width{};           //!< Width of the image in pixels
    std::uint32_t height{};          //!< Height of the image in pixels (number of rows)
    std::uint32_t format{};          //!< driver backend-specific pixel format
    WallClock::TimePoint timestamp;  //!< image acquistion timestamp
  };
  Header header;
  std::span<std::byte> pixels;  //!< pixel data
};

/// @return true if image dimensions and format matches
constexpr auto matchesFormat(const ImageFrame::Header& hdr1, const ImageFrame::Header& hdr2)
    -> bool {
  return std::tie(hdr1.width, hdr1.height, hdr1.format) ==
         std::tie(hdr2.width, hdr2.height, hdr2.format);
}

/// Save an image frame to a file
/// @param frame The image frame to save
/// @param fname The file path to save the image frame to
/// @return true if the image frame was saved successfully, false otherwise
[[nodiscard]] auto save(const ImageFrame& frame, const std::filesystem::path& fname) -> bool;

}  // namespace grape::camera
