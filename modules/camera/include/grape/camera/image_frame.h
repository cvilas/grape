//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>
#include <filesystem>
#include <span>

#include "grape/camera/image_spec.h"
#include "grape/wall_clock.h"

namespace grape::camera {

//=================================================================================================
/// Single image frame data
struct ImageFrame {
  struct Header {
    WallClock::TimePoint timestamp;  //!< Acquistion timestamp
    std::uint32_t bytes_pitch{};     //!< Bytes per row of pixels
    ImageSpec image_spec{};          //!< image data specification
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
