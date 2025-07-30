//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <functional>
#include <vector>

#include "grape/camera/image_frame.h"

namespace grape::camera {

//=================================================================================================
/// Converts image to a format suitable for network transmission
class Formatter {
public:
  struct Stats {
    std::size_t src_size;
    std::size_t dst_size;
  };
  using Callback = std::function<void(const ImageFrame&, const Stats&)>;

  explicit Formatter(Callback&& callback);

  /// Conversion function
  /// @param src Source image frame to convert
  /// @return true if conversion and callback succeeded, false otherwise
  auto format(const ImageFrame& src) -> bool;

private:
  std::vector<std::byte> buffer_;
  Callback callback_;
};

}  // namespace grape::camera
