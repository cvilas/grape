//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <SDL3/SDL_surface.h>

namespace grape::camera {
auto calcPixelBufferSize(const SDL_Surface* surf) -> int;
}
