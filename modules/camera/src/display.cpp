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

  auto& texture = impl_->texture;
  auto& renderer = impl_->renderer;
  const auto iw = static_cast<int>(header.width);
  const auto ih = static_cast<int>(header.height);
  if ((texture == nullptr) || (texture->w != iw) || (texture->h != ih) ||
      (texture->format != header.format)) {
    texture.reset(SDL_CreateTexture(renderer.get(), static_cast<SDL_PixelFormat>(header.format),
                                    SDL_TEXTUREACCESS_STREAMING, iw, ih));
    if (texture == nullptr) {
      syslog::Error("SDL_CreateTexture failed: {}", SDL_GetError());
      return;
    }
  }

  void* pixels = nullptr;
  int pitch = 0;
  if (not SDL_LockTexture(texture.get(), nullptr, &pixels, &pitch)) {
    syslog::Error("Failed to lock texture: {}", SDL_GetError());
    return;
  }
  std::memcpy(pixels, frame.pixels.data(), frame.pixels.size_bytes());
  SDL_UnlockTexture(texture.get());

  // Setup background colour
  constexpr auto BGCOLOR = std::array<float, 4>{ 0.4F, 0.6F, 1.0F, SDL_ALPHA_OPAQUE_FLOAT };
  if (not SDL_SetRenderDrawColorFloat(renderer.get(), BGCOLOR.at(0), BGCOLOR.at(1), BGCOLOR.at(2),
                                      BGCOLOR.at(3))) {
    syslog::Warn("Failed to draw background: {}", SDL_GetError());
  }
  if (not SDL_RenderClear(renderer.get())) {
    syslog::Warn("Failed to clear renderer: {}", SDL_GetError());
  }

  auto dst = SDL_Rect{};
  auto fdst = SDL_FRect{};
  SDL_GetRenderOutputSize(renderer.get(), &dst.w, &dst.h);
  SDL_RectToFRect(&dst, &fdst);

  // Calculate view dimensions maintaining aspect ratio
  const auto src_w = static_cast<float>(texture->w);
  const auto src_h = static_cast<float>(texture->h);
  const auto scale_w = fdst.w / src_w;
  const auto scale_h = fdst.h / src_h;
  const auto scale = std::min(scale_w, scale_h);
  const auto new_w = src_w * scale;
  const auto new_h = src_h * scale;
  fdst.x = (fdst.w - new_w) / 2;
  fdst.y = (fdst.h - new_h) / 2;
  fdst.w = new_w;
  fdst.h = new_h;

  if (not SDL_RenderTexture(renderer.get(), texture.get(), nullptr, &fdst)) {
    syslog::Warn("Failed to render texture: {}", SDL_GetError());
  }

  if (not SDL_RenderPresent(renderer.get())) {
    syslog::Warn("Failed to preset image: {}", SDL_GetError());
  }
}

}  // namespace grape::camera
