//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <span>

namespace grape::camera {

//=================================================================================================
/// Single image frame data
struct ImageFrame {
  struct Header {
    std::uint32_t pitch{};   //!< Bytes per row of pixels
    std::uint32_t width{};   //!< Width of the image in pixels
    std::uint32_t height{};  //!< Height of the image in pixels (number of rows)
    std::uint32_t format{};  //!< driver backend-specific pixel format
    std::chrono::system_clock::time_point timestamp;  //!< image acquistion timestamp
  };
  Header header;
  std::span<std::byte> pixels;  //!< pixel data
};

/// Save an image frame to a file
/// @param frame The image frame to save
/// @param fname The file path to save the image frame to
/// @return true if the image frame was saved successfully, false otherwise
[[nodiscard]] auto save(const ImageFrame& frame, const std::filesystem::path& fname) -> bool;

}  // namespace grape::camera
