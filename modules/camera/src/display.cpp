//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/camera/display.h"

#include <SDL3/SDL_render.h>

#include "grape/exception.h"
#include "grape/log/syslog.h"

namespace grape::camera {

struct Display::Impl {
  std::unique_ptr<SDL_Window, void (*)(SDL_Window*)> window{ nullptr, SDL_DestroyWindow };
  std::unique_ptr<SDL_Renderer, void (*)(SDL_Renderer*)> renderer{ nullptr, SDL_DestroyRenderer };
  std::unique_ptr<SDL_Texture, void (*)(SDL_Texture*)> texture{ nullptr, SDL_DestroyTexture };
};

//-------------------------------------------------------------------------------------------------
Display::Display() : impl_(std::make_unique<Impl>()) {
  static constexpr auto DEFAULT_WIDTH = 640;
  static constexpr auto DEFAULT_HEIGHT = 480;

  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;

  if (not SDL_CreateWindowAndRenderer("Camera View", DEFAULT_WIDTH, DEFAULT_HEIGHT,
                                      SDL_WINDOW_RESIZABLE, &window, &renderer)) {
    panic<Exception>(std::format("SDL_CreateWindowAndRenderer failed: {}", SDL_GetError()));
  }

  impl_->window.reset(window);
  impl_->renderer.reset(renderer);
}

//-------------------------------------------------------------------------------------------------
Display::~Display() = default;

//-------------------------------------------------------------------------------------------------
void Display::render(const ImageFrame& frame) {
  const auto& header = frame.header;
  auto sdl_frame = std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)>(
      SDL_CreateSurfaceFrom(static_cast<int>(header.width), static_cast<int>(header.height),
                            static_cast<SDL_PixelFormat>(header.format), frame.pixels.data(),
                            static_cast<int>(header.pitch)),
      SDL_DestroySurface);
  if (sdl_frame == nullptr) {
    syslog::Warn("Could not create SDL surface: {}", SDL_GetError());
    return;
  }

  // Create or recreate texture if needed
  auto& texture = impl_->texture;
  auto& renderer = impl_->renderer;
  if (texture == nullptr || texture->w != sdl_frame->w || texture->h != sdl_frame->h ||
      texture->format != sdl_frame->format) {
    texture.reset(SDL_CreateTexture(renderer.get(), sdl_frame->format, SDL_TEXTUREACCESS_STREAMING,
                                    sdl_frame->w, sdl_frame->h));
    if (texture == nullptr) {
      syslog::Error("SDL_CreateTexture failed: {}", SDL_GetError());
      return;
    }
  }

  // Update texture with new frame data
  if (not SDL_UpdateTexture(texture.get(), nullptr, sdl_frame->pixels, sdl_frame->pitch)) {
    syslog::Error("SDL_UpdateTexture failed: {}", SDL_GetError());
  }

  // Setup background colour
  constexpr auto BGCOLOR = std::array<float, 4>{ 0.4F, 0.6F, 1.0F, SDL_ALPHA_OPAQUE_FLOAT };
  if (not SDL_SetRenderDrawColorFloat(renderer.get(), BGCOLOR.at(0), BGCOLOR.at(1), BGCOLOR.at(2),
                                      BGCOLOR.at(3))) {
    syslog::Warn("SDL_SetRenderDrawColorFloat failed: {}", SDL_GetError());
  }

  if (not SDL_RenderClear(renderer.get())) {
    syslog::Warn("SDL_RenderClear failed: {}", SDL_GetError());
  }

  int dw = 0;
  int dh = 0;
  SDL_GetRenderOutputSize(renderer.get(), &dw, &dh);
  auto dst = SDL_FRect{ .x = 0, .y = 0, .w = static_cast<float>(dw), .h = static_cast<float>(dh) };

  // Calculate view dimensions maintaining aspect ratio
  const auto src_w = static_cast<float>(texture->w);
  const auto src_h = static_cast<float>(texture->h);
  const auto scale_w = dst.w / src_w;
  const auto scale_h = dst.h / src_h;
  const auto scale = std::min(scale_w, scale_h);
  const auto new_w = src_w * scale;
  const auto new_h = src_h * scale;
  dst.x = (dst.w - new_w) / 2;
  dst.y = (dst.h - new_h) / 2;
  dst.w = new_w;
  dst.h = new_h;

  if (not SDL_RenderTexture(renderer.get(), texture.get(), nullptr, &dst)) {
    syslog::Warn("SDL_RenderTexture failed: {}", SDL_GetError());
  }

  if (not SDL_RenderPresent(renderer.get())) {
    syslog::Warn("SDL_RenderPresent failed: {}", SDL_GetError());
  }
}

}  // namespace grape::camera
