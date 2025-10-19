//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <functional>
#include <memory>
#include <string>

#include "grape/camera/image_frame.h"

namespace grape::picam {

//=================================================================================================
/// Camera image capture interface using libcamera
class PiCamera {
public:
  using Callback = std::function<void(const camera::ImageFrame& frame)>;

  /// Initialise a camera
  /// @param callback Callback to be invoked when a new image frame is available
  /// @param name_hint User-provided hint for camera to open by specifying a part of its name. If
  /// not specified, the first enumerated camera is chosen
  explicit PiCamera(Callback&& callback, const std::string& name_hint = "");

  /// Acquire an image. Trigger callback (set in the constructor) when an image becomes available
  void acquire();

  ~PiCamera();
  PiCamera(const PiCamera&) = delete;
  PiCamera(PiCamera&&) = delete;
  auto operator=(const PiCamera&) = delete;
  auto operator=(PiCamera&&) = delete;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
  Callback callback_;
};
}  // namespace grape::picam
