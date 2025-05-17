//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#define SDL_MAIN_USE_CALLBACKS 1  // NOLINT(cppcoreguidelines-macro-usage)

#include <atomic>
#include <chrono>
#include <cmath>
#include <memory>

#include <SDL3/SDL_camera.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>

#include "grape/log/syslog.h"
#include "grape/utils/format_ranges.h"

//-------------------------------------------------------------------------------------------------
// Demonstrates using SDL3 camera API to acquire and show camera frames.
// Set SDL_CAMERA_DRIVER environment variable to specify a driver backend.
// Eg: SDL_CAMERA_DRIVER=v4l2 grape_camera_view
//-------------------------------------------------------------------------------------------------

namespace {

//=================================================================================================
/// Encapsulates data and model for statistics calculations
class Statistics {
public:
  void compute();

private:
  static constexpr auto REPORT_INTERVAL = std::chrono::seconds(30);
  uint64_t frame_count_{ 0 };
  std::chrono::system_clock::time_point start_ts_;
};

//=================================================================================================
// Encapsulates application state
struct AppState {
  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;
  SDL_Camera* camera = nullptr;
  SDL_Texture* texture = nullptr;
  std::atomic_bool take_snapshot = false;
  Statistics stats;
};

//-------------------------------------------------------------------------------------------------
void Statistics::compute() {
  const auto ts = std::chrono::system_clock::now();
  if (frame_count_ == 0) {
    start_ts_ = ts;
  }
  frame_count_++;
  if (ts - start_ts_ > REPORT_INTERVAL) {
    const auto stop = std::chrono::system_clock::now();
    const auto dt = std::chrono::duration<double>(stop - start_ts_).count();
    const auto frame_rate = std::floor(static_cast<double>(frame_count_) / dt);
    grape::syslog::Info("[{} frames/s (avg. over {})]", frame_rate, REPORT_INTERVAL);
    frame_count_ = 0;
  }
}

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

//-------------------------------------------------------------------------------------------------
// Get list of supported backends
auto getCameraDrivers() -> std::vector<std::string> {
  const auto num_drivers = SDL_GetNumCameraDrivers();
  std::vector<std::string> drivers;
  drivers.reserve(static_cast<std::size_t>(num_drivers));
  for (int i = 0; i < num_drivers; ++i) {
    drivers.emplace_back(SDL_GetCameraDriver(i));
  }
  return drivers;
}

//-------------------------------------------------------------------------------------------------
// Print camera name and supported formats
void printCameraSpecs(SDL_CameraID camera_id) {
  const auto* name = SDL_GetCameraName(camera_id);
  grape::syslog::Info(" {}", name);
  SDL_CameraSpec** specs = SDL_GetCameraSupportedFormats(camera_id, nullptr);
  if (specs == nullptr) {
    grape::syslog::Warn("Unable to get camera specs: {}", SDL_GetError());
    return;
  }
  grape::syslog::Info("  Supported specs:");
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  for (int i = 0; specs[i] != nullptr; ++i) {
    const auto* sp = specs[i];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    grape::syslog::Info("   {}x{}, {} FPS, {}", sp->width, sp->height,
                        sp->framerate_numerator / sp->framerate_denominator,
                        SDL_GetPixelFormatName(sp->format));
  }
  // NOLINTNEXTLINE(cppcoreguidelines-no-malloc,cppcoreguidelines-owning-memory,bugprone-multi-level-implicit-pointer-conversion)
  SDL_free(specs);
}

}  // namespace

//-------------------------------------------------------------------------------------------------
auto SDL_AppInit(void** appstate, int /*argc*/, char** /*argv*/) -> SDL_AppResult {
  setupLogging();

  auto app_state_ptr = std::make_unique<AppState>();
  *appstate = app_state_ptr.get();

  if (not SDL_Init(SDL_INIT_VIDEO | SDL_INIT_CAMERA)) {
    grape::syslog::Critical("SDL_Init failed: {}", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  grape::syslog::Info("Available camera drivers: {}", getCameraDrivers());
  grape::syslog::Info("Using camera driver: {}", SDL_GetCurrentCameraDriver());

  auto camera_count = 0;
  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,cppcoreguidelines-no-malloc)
  auto camera_ids_deleter = [](SDL_CameraID* ptr) { SDL_free(ptr); };
  auto cameras_ids = std::unique_ptr<SDL_CameraID, decltype(camera_ids_deleter)>(
      SDL_GetCameras(&camera_count), camera_ids_deleter);
  if ((cameras_ids == nullptr) or (camera_count == 0)) {
    grape::syslog::Critical("No cameras found, or unable to enumerate cameras: {}", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  grape::syslog::Info("Found {} camera{}", camera_count, camera_count > 1 ? "s" : "");
  for (int i = 0; i < camera_count; ++i) {
    const auto id = cameras_ids.get()[i];
    printCameraSpecs(id);
  }

  grape::syslog::Info("Opening first camera");
  app_state_ptr->camera = SDL_OpenCamera(cameras_ids.get()[0], nullptr);
  if (app_state_ptr->camera == nullptr) {
    grape::syslog::Critical("Unable to open camera: {}", SDL_GetError());
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

  return SDL_APP_CONTINUE;
}

//-------------------------------------------------------------------------------------------------
auto SDL_AppIterate(void* appstate) -> SDL_AppResult {
  auto* app_state_ptr = static_cast<AppState*>(appstate);

  auto* frame = SDL_AcquireCameraFrame(app_state_ptr->camera, nullptr /*timestamp*/);
  if (frame == nullptr) {
    return SDL_APP_CONTINUE;
  }

  app_state_ptr->stats.compute();

  if (app_state_ptr->texture == nullptr) {
    app_state_ptr->texture = SDL_CreateTexture(app_state_ptr->renderer, frame->format,
                                               SDL_TEXTUREACCESS_STREAMING, frame->w, frame->h);
  }

  if (not SDL_UpdateTexture(app_state_ptr->texture, nullptr, frame->pixels, frame->pitch)) {
    grape::syslog::Warn("SDL_UpdateTexture failed: {}", SDL_GetError());
  }

  if (app_state_ptr->take_snapshot) {
    app_state_ptr->take_snapshot = false;
    auto fname = std::format("snapshot_{:%FT%T}.bmp", std::chrono::system_clock::now());
    if (not SDL_SaveBMP(frame, fname.c_str())) {
      grape::syslog::Warn("Could not save snapshot");
    } else {
      grape::syslog::Info("Saved snapshot to {}", fname);
    }
  }
  SDL_ReleaseCameraFrame(app_state_ptr->camera, frame);

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
auto SDL_AppEvent(void* appstate, SDL_Event* event) -> SDL_AppResult {
  auto* app_state_ptr = static_cast<AppState*>(appstate);

  if (event->type == SDL_EVENT_QUIT) {
    grape::syslog::Info("Quit!");
    return SDL_APP_SUCCESS;
  }

  if (event->type == SDL_EVENT_CAMERA_DEVICE_DENIED) {
    grape::syslog::Error("Camera access denied!");
    return SDL_APP_FAILURE;
  }

  if (event->type == SDL_EVENT_CAMERA_DEVICE_APPROVED) {
    grape::syslog::Info("Camera access approved!");
    auto spec = SDL_CameraSpec{};
    if (not SDL_GetCameraFormat(app_state_ptr->camera, &spec)) {
      grape::syslog::Warn("Unable to get camera format: {}", SDL_GetError());
    }
    const auto fps = spec.framerate_numerator / spec.framerate_denominator;
    grape::syslog::Info("Camera Spec: {}x{}, {} FPS, {}", spec.width, spec.height, fps,
                        SDL_GetPixelFormatName(spec.format));
    return SDL_APP_CONTINUE;
  }

  if (event->type == SDL_EVENT_KEY_DOWN) {
    if (event->key.scancode == SDL_SCANCODE_S) {
      app_state_ptr->take_snapshot = true;
    }
    return SDL_APP_CONTINUE;
  }

  return SDL_APP_CONTINUE;
}

//-------------------------------------------------------------------------------------------------
void SDL_AppQuit(void* appstate, SDL_AppResult /*result*/) {
  auto* app_state_ptr = static_cast<AppState*>(appstate);
  if (app_state_ptr->camera != nullptr) {
    SDL_CloseCamera(app_state_ptr->camera);
  }
  if (app_state_ptr->texture != nullptr) {
    SDL_DestroyTexture(app_state_ptr->texture);
  }
}
