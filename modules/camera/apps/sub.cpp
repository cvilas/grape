//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#define SDL_MAIN_USE_CALLBACKS 1  // NOLINT(cppcoreguidelines-macro-usage)

#include <atomic>
#include <chrono>
#include <cmath>

#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>

#include "grape/log/syslog.h"
#include "grape/utils/format_ranges.h"

//-------------------------------------------------------------------------------------------------
// Demonstrates using SDL3 camera API to acquire and show camera frames.
// Set SDL_CAMERA_DRIVER environement variable to specify a driver backend.
// Eg: SDL_CAMERA_DRIVER=v4l2 grape_camera_view
//-------------------------------------------------------------------------------------------------

namespace {

//=================================================================================================
// Encapsulates application state
struct AppState {
  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;
  SDL_Texture* texture = nullptr;
};

//-------------------------------------------------------------------------------------------------
struct LogFormatter {
  static auto format(const grape::log::Record& record) -> std::string {
    return std::format("[{}] [{:9s}] {}", record.timestamp, toString(record.severity),
                       record.message.cStr());
  }
};

//-------------------------------------------------------------------------------------------------
void setupLogging() {
  auto log_config = grape::log::Config{};
  log_config.sink = std::make_shared<grape::log::ConsoleSink<LogFormatter>>();
  log_config.threshold = grape::log::Severity::Info;
  grape::syslog::init(std::move(log_config));
}

}  // namespace

//-------------------------------------------------------------------------------------------------
auto SDL_AppInit(void** appstate, int /*argc*/, char** /*argv*/) -> SDL_AppResult {
  setupLogging();

  auto app_state_ptr = std::make_unique<AppState>();
  *appstate = app_state_ptr.get();

  if (not SDL_Init(SDL_INIT_VIDEO)) {
    grape::syslog::Critical("SDL_Init failed: {}", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  static constexpr auto DEFAULT_WIDTH = 640;
  static constexpr auto DEFAULT_HEIGHT = 480;
  if (not SDL_CreateWindowAndRenderer("Camera View", DEFAULT_WIDTH, DEFAULT_HEIGHT,
                                      SDL_WINDOW_RESIZABLE, &(app_state_ptr->window),
                                      &(app_state_ptr->renderer))) {
    grape::syslog::Critical("SDL_CreateWindowAndRenderer failed: {}", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Transfer ownership to a static storage for automatic cleanup on exit
  static auto app_state_holder = std::move(app_state_ptr);

  // TODO: Start receiver thread

  return SDL_APP_CONTINUE;
}

//-------------------------------------------------------------------------------------------------
auto SDL_AppIterate(void* appstate) -> SDL_AppResult {
  auto* app_state_ptr = static_cast<AppState*>(appstate);

  // TODO: Pull frame from ring buffer
  SDL_Surface* frame = nullptr;

  if (app_state_ptr->texture == nullptr) {
    app_state_ptr->texture = SDL_CreateTexture(app_state_ptr->renderer, frame->format,
                                               SDL_TEXTUREACCESS_STREAMING, frame->w, frame->h);
  }

  if (not SDL_UpdateTexture(app_state_ptr->texture, nullptr, frame->pixels, frame->pitch)) {
    grape::syslog::Warn("SDL_UpdateTexture failed: {}", SDL_GetError());
  }

  constexpr auto BGCOLOR = std::array<float, 4>{ 0.4F, 0.6F, 1.0F, SDL_ALPHA_OPAQUE_FLOAT };
  if (not SDL_SetRenderDrawColorFloat(app_state_ptr->renderer, BGCOLOR.at(0), BGCOLOR.at(1),
                                      BGCOLOR.at(2), BGCOLOR.at(3))) {
    grape::syslog::Warn("SDL_SetRenderDrawColorFloat failed: {}", SDL_GetError());
  }

  if (not SDL_RenderClear(app_state_ptr->renderer)) {
    grape::syslog::Warn("SDL_RenderClear failed: {}", SDL_GetError());
  }

  int dw = 0;
  int dh = 0;
  SDL_GetRenderOutputSize(app_state_ptr->renderer, &dw, &dh);
  auto dst = SDL_FRect{ .x = 0, .y = 0, .w = static_cast<float>(dw), .h = static_cast<float>(dh) };

  if (app_state_ptr->texture != nullptr) {
    // Calculate view dimensions maintaining aspect ratio
    const auto src_w = static_cast<float>(app_state_ptr->texture->w);
    const auto src_h = static_cast<float>(app_state_ptr->texture->h);
    const auto scale_w = dst.w / src_w;
    const auto scale_h = dst.h / src_h;
    const auto scale = std::min(scale_w, scale_h);
    const auto new_w = src_w * scale;
    const auto new_h = src_h * scale;
    dst.x = (dst.w - new_w) / 2;
    dst.y = (dst.h - new_h) / 2;
    dst.w = new_w;
    dst.h = new_h;

    if (not SDL_RenderTexture(app_state_ptr->renderer, app_state_ptr->texture, nullptr, &dst)) {
      grape::syslog::Warn("SDL_RenderTexture failed: {}", SDL_GetError());
    }
  }

  if (not SDL_RenderPresent(app_state_ptr->renderer)) {
    grape::syslog::Warn("SDL_RenderPresent failed: {}", SDL_GetError());
  }

  return SDL_APP_CONTINUE;
}

//-------------------------------------------------------------------------------------------------
auto SDL_AppEvent(void* /*appstate*/, SDL_Event* event) -> SDL_AppResult {
  if (event->type == SDL_EVENT_QUIT) {
    grape::syslog::Info("Quit!");
    return SDL_APP_SUCCESS;
  }
  return SDL_APP_CONTINUE;
}

//-------------------------------------------------------------------------------------------------
void SDL_AppQuit(void* appstate, SDL_AppResult /*result*/) {
  auto* app_state_ptr = static_cast<AppState*>(appstate);
  if (app_state_ptr->texture != nullptr) {
    SDL_DestroyTexture(app_state_ptr->texture);
  }
  // TODO: Stop receiver thread
}
