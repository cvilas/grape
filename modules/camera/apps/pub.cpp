//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

// capture frame -> ring_buffer -> compression -> pub
//   - main thread captures and pushes frame into ring buffer
//   - pub thread pops frame, compresses it and publishes it
// sub -> decompress frame -> ring_buffer -> view (fps controlled)
//   - sub thread receives frame and pushes it into ring buffer
//   - main thread pops frame, decompresses it and displays it with the right delay (SDL operations
//     should be on main thread)

// TODO
// - command line interface to start and stop capture
// - configuration file to set
//   - camera device
//   - select camera spec (width, height, format, fps)
//   - compression parameters
//   - publisher parameters (topic name, etc)
// - publish per session metadata
//   - camera name
//   - camera spec
//   - compression parameters
// - Include per-frame information
//   - absolute capture timestamp
//   - frame number
// - code organisation
//   - capture thread (main) captures and pushes frames to ring buffer
//   - publish thread pops frame from ring buffer, compresses it, publishes it
// - Option for HW jpeg encoding (instead of lz4)

#define SDL_MAIN_USE_CALLBACKS 1  // NOLINT(cppcoreguidelines-macro-usage)

#include <atomic>
#include <chrono>
#include <cmath>
#include <memory>

#include <SDL3/SDL_camera.h>
#include <SDL3/SDL_main.h>

#include "grape/log/syslog.h"
#include "grape/utils/format_ranges.h"

//-------------------------------------------------------------------------------------------------
// Demonstrates using SDL3 camera API to acquire and publish camera frames using IPC.
//-------------------------------------------------------------------------------------------------

namespace {

//=================================================================================================
// Encapsulates application state
struct AppState {
  SDL_Camera* camera = nullptr;
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

  if (not SDL_Init(SDL_INIT_CAMERA)) {
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

  // TODO: Setup publisher thread

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

  // TODO: push frame into ring buffer

  SDL_ReleaseCameraFrame(app_state_ptr->camera, frame);

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

  return SDL_APP_CONTINUE;
}

//-------------------------------------------------------------------------------------------------
void SDL_AppQuit(void* appstate, SDL_AppResult /*result*/) {
  auto* app_state_ptr = static_cast<AppState*>(appstate);
  if (app_state_ptr->camera != nullptr) {
    SDL_CloseCamera(app_state_ptr->camera);
  }
  // TODO: Stop publish thread
}
