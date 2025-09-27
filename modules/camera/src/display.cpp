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
  [[nodiscard]] auto mean() const -> float {
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

  const auto num_supported_renderers = SDL_GetNumRenderDrivers();
  auto supported_renderers = std::vector<std::string_view>{};
  for (int i = 0; i < num_supported_renderers; ++i) {
    supported_renderers.emplace_back(SDL_GetRenderDriver(i));
  }
  syslog::Info("Supported renderers: {}", supported_renderers);

  if (not SDL_CreateWindowAndRenderer("Camera View", DEFAULT_WIDTH, DEFAULT_HEIGHT,
                                      SDL_WINDOW_RESIZABLE, &window, &renderer)) {
    panic(std::format("SDL_CreateWindowAndRenderer failed: {}", SDL_GetError()));
  }
  syslog::Note("Using renderer: {}", SDL_GetRendererName(renderer));

  impl_->window.reset(window);
  impl_->renderer.reset(renderer);
}

//-------------------------------------------------------------------------------------------------
Display::~Display() = default;

//-------------------------------------------------------------------------------------------------
void Display::showTimestamp(bool en) {
  show_timestamp_ = en;
}

//-------------------------------------------------------------------------------------------------
void Display::render(const ImageFrame& frame) {
  const auto& header = frame.header;
  auto* renderer = impl_->renderer.get();

  // initialise texture to render image into
  static auto previous_image_header = ImageFrame::Header{};
  if (not matchesFormat(previous_image_header, header)) {
    previous_image_header = header;
    const auto fmt = static_cast<SDL_PixelFormat>(header.format);
    const auto iw = static_cast<int>(header.width);
    const auto ih = static_cast<int>(header.height);
    syslog::Note("Receiving {}x{}, {}", iw, ih, SDL_GetPixelFormatName(fmt));
    impl_->texture.reset(SDL_CreateTexture(renderer, fmt, SDL_TEXTUREACCESS_STREAMING, iw, ih));
    if (impl_->texture == nullptr) {
      syslog::Error("Failed to create texture: {}", SDL_GetError());
      return;
    }

    // maintain aspect ratio as window size changes
    if (not SDL_SetRenderLogicalPresentation(renderer, iw, ih,
                                             SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
      syslog::Warn("Failed to set presentation mode: {}", SDL_GetError());
    }
  }

  // render image into texture
  auto* texture = impl_->texture.get();
  if (not SDL_UpdateTexture(texture, nullptr, frame.pixels.data(),
                            static_cast<int>(header.pitch))) {
    syslog::Warn("Failed to update texture: {}", SDL_GetError());
  }
  if (not SDL_RenderTexture(renderer, texture, nullptr, nullptr)) {
    syslog::Warn("Failed to render texture: {}", SDL_GetError());
  }

  // render OSD
  if (show_timestamp_) {
    // set font color
    static constexpr auto COLOR = std::array<Uint8, 4>{ 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE };
    if (!SDL_SetRenderDrawColor(renderer, COLOR.at(0), COLOR.at(1), COLOR.at(2), COLOR.at(3))) {
      syslog::Warn("Failed to set font color: {}", SDL_GetError());
    }

    // Determine image scaling and invert it to scale text such that its size appears constant
    auto window_w = 1;
    auto window_h = 1;
    if (!SDL_GetRenderOutputSize(renderer, &window_w, &window_h)) {
      syslog::Warn("Failed to get window size: {}", SDL_GetError());
    }
    const auto scale_x = static_cast<float>(window_w) / static_cast<float>(header.width);
    const auto scale_y = static_cast<float>(window_h) / static_cast<float>(header.height);
    const auto ts_scale = 1.F / ((scale_x < scale_y) ? scale_x : scale_y);

    // print timestamp
    static constexpr auto TS_X = 10;
    static constexpr auto TS_Y = 10;
    const auto ts_text = std::format("{}", header.timestamp);
    if (!SDL_SetRenderScale(renderer, ts_scale, ts_scale)) {
      syslog::Warn("Failed to scale font: {}", SDL_GetError());
    }
    if (!SDL_RenderDebugText(renderer, TS_X, TS_Y, ts_text.c_str())) {
      syslog::Warn("Failed to render text: {}", SDL_GetError());
    }
    if (!SDL_SetRenderScale(renderer, 1.0F, 1.0F)) {
      syslog::Warn("Failed to reset font scale: {}", SDL_GetError());
    }
  }

  // show image
  const auto now = SystemClock::now();
  if (not SDL_RenderPresent(renderer)) {
    syslog::Warn("Failed to present image: {}", SDL_GetError());
  }
  const auto dt = now - header.timestamp;
  impl_->latency_accum.append(std::chrono::duration_cast<std::chrono::duration<float>>(dt).count());
}

//-------------------------------------------------------------------------------------------------
auto Display::latency() const -> SystemClock::Duration {
  const auto mean_latency = impl_->latency_accum.mean();
  impl_->latency_accum.reset();
  return std::chrono::duration_cast<SystemClock::Duration>(
      std::chrono::duration<float>(mean_latency));
}

}  // namespace grape::camera
