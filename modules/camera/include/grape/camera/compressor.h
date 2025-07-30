//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <functional>
#include <span>
#include <vector>

#include "grape/camera/image_frame.h"

namespace grape::camera {

//=================================================================================================
/// Compresses an image frame without loss into byte stream for network transmission
class Compressor {
public:
  struct Stats {
    std::size_t src_size;
    std::size_t dst_size;
  };
  using Callback = std::function<void(std::span<const std::byte>, const Stats&)>;

  /// Constructor
  /// @param cb callback to trigger when compressed image is available
  explicit Compressor(Callback&& cb);

  /// Compress an image and trigger callback when done
  /// @param image Source image to compress
  /// @return true on success
  [[nodiscard]] auto compress(const ImageFrame& image) -> bool;

private:
  std::vector<std::byte> buffer_;  //!< compressed data buffer
  Callback callback_;
};
}  // namespace grape::camera
