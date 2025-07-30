//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/camera/image_frame.h"

#include <SDL3/SDL_surface.h>

#include "grape/log/syslog.h"

namespace grape::camera {

//-------------------------------------------------------------------------------------------------
auto save(const ImageFrame& frame, const std::filesystem::path& fname) -> bool {
  const auto& header = frame.header;
  auto sdl_frame = std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)>(
      SDL_CreateSurfaceFrom(static_cast<int>(header.width), static_cast<int>(header.height),
                            static_cast<SDL_PixelFormat>(header.format), frame.pixels.data(),
                            static_cast<int>(header.pitch)),
      SDL_DestroySurface);

  if (sdl_frame == nullptr) {
    syslog::Warn("Failed to create SDL surface: {}", SDL_GetError());
    return false;
  }
  if (not SDL_SaveBMP(sdl_frame.get(), fname.c_str())) {
    syslog::Warn("Failed to save snapshot: {}", SDL_GetError());
    return false;
  }
  syslog::Info("Saved snapshot to {}", fname.string());
  return true;
}
}  // namespace grape::camera
