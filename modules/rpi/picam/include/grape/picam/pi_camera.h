//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <functional>
#include <memory>
#include <string>

#include "grape/camera/image_frame.h"
#include "grape/camera/image_spec.h"

namespace grape::camera {

//=================================================================================================
/// Camera image capture interface using libcamera
class PiCamera {
public:
  using Callback = std::function<void(const camera::ImageFrame& frame)>;

  struct Config {
    static constexpr auto DEFAULT_IMAGE_SIZE = ImageSize{ .width = 1920U, .height = 1080U };

    /// Camera name. Partial name (hint) is supported. If unspecified, choose first camera
    std::string camera_name_hint;

    /// Target resolution (pixels). Camera will select closest matching resolution
    ImageSize image_size{ DEFAULT_IMAGE_SIZE };
  };

  /// Initialise a camera
  /// @param config Camera capture configuration
  /// @param image_callback Callback to trigger on image capture
  PiCamera(const Config& config, Callback&& image_callback);

  /// Acquire an image. Trigger callback when an image becomes available
  void acquire();

  ~PiCamera();
  PiCamera(const PiCamera&) = delete;
  PiCamera(PiCamera&&) = delete;
  auto operator=(const PiCamera&) = delete;
  auto operator=(PiCamera&&) = delete;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};
}  // namespace grape::camera
