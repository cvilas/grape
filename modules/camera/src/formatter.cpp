//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/camera/formatter.h"

#include <memory>

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_surface.h>

#include "grape/log/syslog.h"

namespace grape::camera {

//-------------------------------------------------------------------------------------------------
Formatter::Formatter(Callback&& callback) : callback_(std::move(callback)) {
}

//-------------------------------------------------------------------------------------------------
auto Formatter::format(const ImageFrame& src) -> bool {
  static constexpr auto TARGET_FMT = SDL_PIXELFORMAT_NV12;
  // Notes:
  // - NV12/YV12 image data size = width * height * 3/2 (Y plane + UV plane)
  // - NV12/YV12 image data pitch = width

  static constexpr auto HDR_SIZE = sizeof(ImageFrame::Header);
  const auto src_size = HDR_SIZE + src.pixels.size_bytes();
  if (src.header.format == TARGET_FMT) {
    callback_(src, { .src_size = src_size, .dst_size = src_size });
    return true;
  }

  auto src_surface = std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)>(
      SDL_CreateSurfaceFrom(static_cast<int>(src.header.width), static_cast<int>(src.header.height),
                            static_cast<SDL_PixelFormat>(src.header.format), src.pixels.data(),
                            static_cast<int>(src.header.pitch)),
      SDL_DestroySurface);

  if (src_surface == nullptr) {
    syslog::Error("Failed to create surface for formatting: {}", SDL_GetError());
    return false;
  }

  const auto reqd_buf_size = (src.header.width * src.header.height * 3) / 2;  // see notes above
  if (buffer_.size() < reqd_buf_size) {
    buffer_.resize(reqd_buf_size);
    syslog::Debug("Formatter buffer resized to {} bytes", reqd_buf_size);
  }
  if (not SDL_ConvertPixels(src_surface->w, src_surface->h, src_surface->format, src.pixels.data(),
                            src_surface->pitch, TARGET_FMT, buffer_.data(), src_surface->w)) {
    syslog::Error("Failed to format: {}", SDL_GetError());
    return false;
  }

  const auto dst = ImageFrame{ .header = { .pitch = src.header.width,
                                           .width = src.header.width,
                                           .height = src.header.height,
                                           .format = static_cast<std::uint32_t>(TARGET_FMT),
                                           .timestamp = src.header.timestamp },
                               .pixels = { buffer_.data(), reqd_buf_size } };

  const auto dst_size = HDR_SIZE + reqd_buf_size;
  callback_(dst, { .src_size = src_size, .dst_size = dst_size });

  return true;
}

}  // namespace grape::camera
