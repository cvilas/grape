//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>

#include "grape/camera/image_frame.h"

namespace grape::camera {

//=================================================================================================
/// Camera image capture interface
class Camera {
public:
  using Callback = std::function<void(const ImageFrame& frame)>;

  /// Initialise a camera
  /// @param callback Callback to be invoked when a new image frame is available
  /// @param name_hint User-provided hint for camera to open by specifying a part of its name. If
  /// not specified, the first enumerated camera is chosen
  explicit Camera(Callback callback, const std::string& name_hint = "");

  /// Acquire an image. Trigger callback (set in the constructor) when an image becomes available
  void acquire();

  ~Camera();
  Camera(const Camera&) = delete;
  Camera(Camera&&) = delete;
  auto operator=(const Camera&) = delete;
  auto operator=(Camera&&) = delete;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
  Callback callback_;
};
}  // namespace grape::camera
