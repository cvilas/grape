//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/camera/display.h"

#include <SDL3/SDL_render.h>

#include "grape/exception.h"
#include "grape/log/syslog.h"
#include "grape/statistics/sliding_mean.h"

namespace grape::camera {

//=================================================================================================
// Sliding mean calculator
class Meanie {
public:
  void append(float value) {
    mean_.store(stats_.append(value, reset_.exchange(false, std::memory_order_relaxed)).mean,
                std::memory_order_relaxed);
  }
  void reset() {
    reset_.store(true, std::memory_order_relaxed);
  }
  auto mean() const -> float {
    return mean_.load(std::memory_order_relaxed);
  }

private:
  static constexpr auto STATS_WINDOW = 600U;
  std::atomic_bool reset_;
  std::atomic<float> mean_;
  statistics::SlidingMean<float, STATS_WINDOW> stats_;
};

//=================================================================================================
struct Display::Impl {
  Meanie latency_accum;
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
  auto* renderer = impl_->renderer.get();

  static auto previous_image_header = ImageFrame::Header{};
  if (not matchesFormat(previous_image_header, header)) {
    previous_image_header = header;
    syslog::Note("Receiving {}x{}, {}", header.width, header.height,
                 SDL_GetPixelFormatName(static_cast<SDL_PixelFormat>(header.format)));

    impl_->texture.reset(SDL_CreateTexture(
        renderer, static_cast<SDL_PixelFormat>(header.format), SDL_TEXTUREACCESS_STREAMING,
        static_cast<int>(header.width), static_cast<int>(header.height)));
    if (impl_->texture == nullptr) {
      syslog::Error("Failed to create texture: {}", SDL_GetError());
      return;
    }
  }

  auto* texture = impl_->texture.get();
  if (not SDL_UpdateTexture(texture, nullptr, frame.pixels.data(),
                            static_cast<int>(header.pitch))) {
    syslog::Error("Failed to update texture:", SDL_GetError());
    return;
  }

  // Setup background colour
  constexpr auto BGCOLOR = std::array<float, 4>{ 0.4F, 0.6F, 1.0F, SDL_ALPHA_OPAQUE_FLOAT };
  if (not SDL_SetRenderDrawColorFloat(renderer, BGCOLOR.at(0), BGCOLOR.at(1), BGCOLOR.at(2),
                                      BGCOLOR.at(3))) {
    syslog::Warn("Failed to draw background: {}", SDL_GetError());
  }
  if (not SDL_RenderClear(renderer)) {
    syslog::Warn("Failed to clear renderer: {}", SDL_GetError());
  }

  auto dst = SDL_Rect{};
  auto fdst = SDL_FRect{};
  SDL_GetRenderOutputSize(renderer, &dst.w, &dst.h);
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

  const auto now = std::chrono::system_clock::now();

  if (not SDL_RenderTexture(renderer, texture, nullptr, &fdst)) {
    syslog::Warn("Failed to render texture: {}", SDL_GetError());
  }

  if (not SDL_RenderPresent(renderer)) {
    syslog::Warn("Failed to present image: {}", SDL_GetError());
  }

  const auto dt =
      std::chrono::duration_cast<std::chrono::duration<float>>(now - header.timestamp).count();
  impl_->latency_accum.append(dt);
}

//-------------------------------------------------------------------------------------------------
auto Display::latency() const -> std::chrono::system_clock::duration {
  const auto mean_latency = impl_->latency_accum.mean();
  impl_->latency_accum.reset();
  return std::chrono::duration_cast<std::chrono::system_clock::duration>(
      std::chrono::duration<float>(mean_latency));
}

}  // namespace grape::camera
