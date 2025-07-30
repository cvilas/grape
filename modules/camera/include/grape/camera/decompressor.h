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
/// Decompress byte stream to image data
class Decompressor {
public:
  using Callback = std::function<void(const ImageFrame&)>;

  /// constructor
  /// @param cb Callback to trigger when decompressed image is available
  explicit Decompressor(Callback&& cb);

  /// Decompress function
  /// @param bytes Source data to decompress
  /// @return true on success
  [[nodiscard]] auto decompress(std::span<const std::byte> bytes) -> bool;

private:
  std::vector<std::byte> buffer_;
  Callback callback_;
};
}  // namespace grape::camera
