//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#define SDL_MAIN_USE_CALLBACKS 1  // NOLINT(cppcoreguidelines-macro-usage)

#include <cmath>

#include <SDL3/SDL_main.h>

#include "grape/camera/camera.h"
#include "grape/camera/display.h"
#include "grape/conio/program_options.h"
#include "grape/exception.h"
#include "grape/log/syslog.h"
#include "grape/time.h"

//-------------------------------------------------------------------------------------------------
// Demonstrates using SDL3 camera API to acquire and show camera frames.
// Set SDL_CAMERA_DRIVER environment variable to specify a driver backend.
// Eg: SDL_CAMERA_DRIVER=v4l2 grape_camera_view
//-------------------------------------------------------------------------------------------------

namespace {

//=================================================================================================
/// Encapsulates processing pipeline
class Application {
public:
  explicit Application(const std::string& camera_name_hint);

  auto iterate() -> SDL_AppResult;
  auto handleEvent(SDL_Event* event) -> SDL_AppResult;
  void saveImage();

private:
  void onCapturedFrame(const grape::camera::ImageFrame& frame);

  grape::camera::Display display_;
  std::unique_ptr<grape::camera::Camera> capture_;
  std::atomic_bool save_snapshot_ = false;
};

//-------------------------------------------------------------------------------------------------
Application::Application(const std::string& camera_name_hint)
  : capture_(std::make_unique<grape::camera::Camera>(
        [this](const auto& frame) { onCapturedFrame(frame); }, camera_name_hint)) {
}

//-------------------------------------------------------------------------------------------------
void Application::onCapturedFrame(const grape::camera::ImageFrame& frame) {
  display_.render(frame);
  if (save_snapshot_) {
    save_snapshot_ = false;
    const auto fname = std::format("snapshot_{:%FT%T}.bmp", grape::SystemClock::now());
    std::ignore = grape::camera::save(frame, fname);
  }
  const auto now = grape::SystemClock::now();
  static auto last_report_ts = now;
  static constexpr auto STATS_REPORT_PERIOD = std::chrono::seconds(10);
  if (now - last_report_ts > STATS_REPORT_PERIOD) {
    last_report_ts = now;
    grape::syslog::Info("Avg. Latency = {}", display_.latency());
  }
}

//-------------------------------------------------------------------------------------------------
void Application::saveImage() {
  save_snapshot_ = true;
}

//-------------------------------------------------------------------------------------------------
auto Application::iterate() -> SDL_AppResult {
  capture_->acquire();
  return SDL_APP_CONTINUE;
}

//-------------------------------------------------------------------------------------------------
auto Application::handleEvent(SDL_Event* event) -> SDL_AppResult {
  if (event->type == SDL_EVENT_QUIT) {
    grape::syslog::Info("Quit!");
    capture_.reset();
    return SDL_APP_SUCCESS;
  }

  if (event->type == SDL_EVENT_CAMERA_DEVICE_DENIED) {
    grape::syslog::Error("Camera access denied!");
    return SDL_APP_FAILURE;
  }

  if (event->type == SDL_EVENT_CAMERA_DEVICE_APPROVED) {
    grape::syslog::Info("Camera access approved!");
    return SDL_APP_CONTINUE;
  }

  if (event->type == SDL_EVENT_KEY_DOWN) {
    if (event->key.scancode == SDL_SCANCODE_S) {
      saveImage();
    }
    return SDL_APP_CONTINUE;
  }

  return SDL_APP_CONTINUE;
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

}  // namespace

//-------------------------------------------------------------------------------------------------
auto SDL_AppInit(void** appstate, int argc, char* argv[]) -> SDL_AppResult {
  try {
    setupLogging();

    if (not SDL_Init(SDL_INIT_VIDEO | SDL_INIT_CAMERA)) {
      grape::syslog::Critical("SDL_Init failed: {}", SDL_GetError());
      return SDL_APP_FAILURE;
    }

    // Parse command line arguments
    const auto args = grape::conio::ProgramDescription("Camera viewer application")
                          .declareOption<std::string>("hint", "Part of camera name to match", "")
                          .parse(argc, const_cast<const char**>(argv));

    const auto camera_name_hint = args.getOption<std::string>("hint");

    auto app = std::make_unique<Application>(camera_name_hint);
    *appstate = app.get();

    // Transfer ownership to static storage for automatic cleanup on exit
    static auto app_holder = std::move(app);

    return SDL_APP_CONTINUE;
  } catch (...) {
    grape::Exception::print();
    return SDL_APP_FAILURE;
  }
}

//-------------------------------------------------------------------------------------------------
auto SDL_AppIterate(void* appstate) -> SDL_AppResult {
  auto* app = static_cast<Application*>(appstate);
  return app->iterate();
}

//-------------------------------------------------------------------------------------------------
auto SDL_AppEvent(void* appstate, SDL_Event* event) -> SDL_AppResult {
  auto* app = static_cast<Application*>(appstate);
  return app->handleEvent(event);
}

//-------------------------------------------------------------------------------------------------
void SDL_AppQuit(void* /*appstate*/, SDL_AppResult /*result*/) {
  // Cleanup is handled automatically by the static app_holder destructor
}
