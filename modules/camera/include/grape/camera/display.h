//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <memory>

#include "grape/camera/image_frame.h"

namespace grape::camera {

//=================================================================================================
/// Display window to render a camera image
class Display {
public:
  /// Display image window
  /// @param frame The image frame to display
  void render(const ImageFrame& frame);

  Display();
  ~Display();
  Display(const Display&) = delete;
  Display(Display&&) = delete;
  auto operator=(const Display&) = delete;
  auto operator=(Display&&) = delete;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace grape::camera
