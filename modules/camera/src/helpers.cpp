//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "helpers.h"

#include "grape/log/syslog.h"

namespace grape::camera {

//-------------------------------------------------------------------------------------------------
auto calcPixelBufferSize(const SDL_Surface* surf) -> int {
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
#endif
  switch (surf->format) {
    case SDL_PIXELFORMAT_NV12:
      [[fallthrough]];
    case SDL_PIXELFORMAT_YV12:
      return (surf->w * surf->h * 3) / 2;
    case SDL_PIXELFORMAT_YUY2:
      return surf->w * surf->h * 2;
    case SDL_PIXELFORMAT_MJPG:
      return surf->w * surf->h;  // TODO(Vilas): likely incorrect. Should be surf->pitch per docs
    case SDL_PIXELFORMAT_RGB24:
      return surf->h * surf->pitch;
    default:
      syslog::Error("Unsupported format {}. Using defaults", SDL_GetPixelFormatName(surf->format));
      return surf->h * surf->pitch;  // safe default
  }
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
}

}  // namespace grape::camera
