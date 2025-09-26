//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/camera/rate_limiter.h"

#include "grape/log/syslog.h"

namespace grape::camera {

//-------------------------------------------------------------------------------------------------
RateLimiter::RateLimiter(std::uint8_t divisor, Callback&& callback)
  : divisor_(divisor > 0U ? divisor : 1U), callback_(std::move(callback)) {
  syslog::Info("Frame rate divisor set to {}", divisor_);
}

//-------------------------------------------------------------------------------------------------
void RateLimiter::process(const ImageFrame& in_frame) {
  if (count_ == 0U) {
    callback_(in_frame);
    count_ = divisor_;
  }
  --count_;
}

}  // namespace grape::camera
