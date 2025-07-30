//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <functional>
#include <span>
#include <vector>

#include "grape/camera/image_frame.h"

namespace grape::camera {

class Compressor {
public:
  using Callback = std::function<void(std::span<const std::byte>)>;
  explicit Compressor(Callback&& cb);
  [[nodiscard]] auto compress(const ImageFrame& image) -> bool;

private:
  std::vector<std::byte> compress_buffer_;
  Callback callback_;
};
}  // namespace grape::camera
