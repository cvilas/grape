//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <functional>

#include "grape/camera/image_frame.h"

namespace grape::camera {

//=================================================================================================
/// Pass through frames at a specified fraction of the input frame rate
class RateLimiter {
public:
  using Callback = std::function<void(const ImageFrame&)>;

  RateLimiter(std::uint8_t divisor, Callback&& callback);

  void process(const ImageFrame& in_frame);

private:
  std::uint8_t divisor_;
  std::uint8_t count_{ 0 };
  Callback callback_;
};

}  // namespace grape::camera
