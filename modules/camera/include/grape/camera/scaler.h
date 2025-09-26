//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <functional>
#include <vector>

#include "grape/camera/image_frame.h"

namespace grape::camera {

//=================================================================================================
/// Scales image by a specified factor maintaining aspect ratio.
class Scaler {
public:
  using Callback = std::function<void(const ImageFrame&)>;

  Scaler(float scale, Callback&& callback);
  auto scale(const ImageFrame& src) -> bool;

private:
  float scale_{ 1.F };
  Callback callback_;
};

}  // namespace grape::camera
