//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/camera/scaler.h"

#include <SDL3/SDL_surface.h>

#include "grape/log/syslog.h"
#include "helpers.h"

namespace grape::camera {

//-------------------------------------------------------------------------------------------------
Scaler::Scaler(float scale, Callback&& callback)
  : scale_(std::abs(scale)), callback_{ std::move(callback) } {
}

//-------------------------------------------------------------------------------------------------
auto Scaler::scale(const ImageFrame& src) -> bool {
  if (std::abs(scale_ - 1.0F) < std::numeric_limits<float>::epsilon()) {
    callback_(src);
    return true;
  }

  // Uses CPU for scaling since ultimately we want to forward data over callback. GPU
  // scaling is inefficient due to slow GPU <-> CPU data transfers
  auto src_surface = std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)>(
      SDL_CreateSurfaceFrom(static_cast<int>(src.header.width), static_cast<int>(src.header.height),
                            static_cast<SDL_PixelFormat>(src.header.format), src.pixels.data(),
                            static_cast<int>(src.header.pitch)),
      SDL_DestroySurface);

  if (src_surface == nullptr) {
    syslog::Error("Failed to create surface for formatting: {}", SDL_GetError());
    return false;
  }

  const auto scaled_h = static_cast<int>(static_cast<float>(src.header.height) * scale_);
  const auto scaled_w = static_cast<int>(static_cast<float>(src.header.width) * scale_);
  auto dst_surface = std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)>(
      SDL_CreateSurface(scaled_w, scaled_h, SDL_PIXELFORMAT_RGB24), SDL_DestroySurface);
  if (dst_surface == nullptr) {
    syslog::Error("Could not create surface for scaling: {}", SDL_GetError());
    return false;
  }

  if (not SDL_BlitSurfaceScaled(src_surface.get(), nullptr, dst_surface.get(), nullptr,
                                SDL_SCALEMODE_LINEAR)) {
    syslog::Error("Could not scale surface: {}", SDL_GetError());
    return false;
  }

  static const auto data_size = calcPixelBufferSize(dst_surface.get());
  const auto frame =
      ImageFrame{ .header = { .pitch = static_cast<std::uint32_t>(dst_surface->pitch),
                              .width = static_cast<std::uint32_t>(dst_surface->w),
                              .height = static_cast<std::uint32_t>(dst_surface->h),
                              .format = static_cast<std::uint32_t>(dst_surface->format),
                              .timestamp = src.header.timestamp },
                  .pixels = { static_cast<std::byte*>(dst_surface->pixels),
                              static_cast<std::size_t>(data_size) } };

  callback_(frame);

  return true;
}

}  // namespace grape::camera
