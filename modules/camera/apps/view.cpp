//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#define SDL_MAIN_USE_CALLBACKS 1  // NOLINT(cppcoreguidelines-macro-usage)

#include <atomic>
#include <chrono>
#include <cmath>
#include <format>
#include <functional>
#include <memory>

#include <SDL3/SDL_camera.h>
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
/// Image frame data structure for passing between capture and display
struct Frame {
  const void* pixels = nullptr;
  int pitch{};
  int width{};
  int height{};
  SDL_PixelFormat format{};
  std::chrono::system_clock::time_point timestamp;
};

using FrameCallback = std::function<void(const Frame& frame)>;

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
/// Handles camera frame acquisition
class Camera {
public:
  explicit Camera(FrameCallback frame_callback);
  auto init() -> bool;     //!< initialise the camera
  auto acquire() -> bool;  //!< acquire a frame
  void save();             //!< save a frame to disk

private:
  static void save(SDL_Surface* frame);
  static void printCameraSpecs(SDL_CameraID camera_id);
  static auto getCameraDrivers() -> std::vector<std::string>;

  std::unique_ptr<SDL_Camera, void (*)(SDL_Camera*)> camera_{ nullptr, SDL_CloseCamera };
  FrameCallback frame_callback_;
  std::atomic_bool save_snapshot_ = false;
  Statistics stats_;
};

//=================================================================================================
/// Handles image rendering and window management
class Display {
public:
  auto init() -> bool;              //!< initialise the display
  void render(const Frame& frame);  //!< show frame

private:
  std::unique_ptr<SDL_Window, void (*)(SDL_Window*)> window_{ nullptr, SDL_DestroyWindow };
  std::unique_ptr<SDL_Renderer, void (*)(SDL_Renderer*)> renderer_{ nullptr, SDL_DestroyRenderer };
  std::unique_ptr<SDL_Texture, void (*)(SDL_Texture*)> texture_{ nullptr, SDL_DestroyTexture };
};

//=================================================================================================
/// Encapsulation of processing pipeline
class Application {
public:
  Application();

  auto init() -> SDL_AppResult;
  auto iterate() -> SDL_AppResult;
  auto handleEvent(SDL_Event* event) -> SDL_AppResult;

private:
  std::unique_ptr<Display> display_;
  std::unique_ptr<Camera> capture_;
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
Camera::Camera(FrameCallback frame_callback) : frame_callback_(std::move(frame_callback)) {
}

//-------------------------------------------------------------------------------------------------
auto Camera::init() -> bool {
  grape::syslog::Info("Available camera drivers: {}", getCameraDrivers());
  grape::syslog::Info("Using camera driver: {}", SDL_GetCurrentCameraDriver());

  auto camera_count = 0;
  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,cppcoreguidelines-no-malloc)
  auto camera_ids_deleter = [](SDL_CameraID* ptr) { SDL_free(ptr); };
  auto cameras_ids = std::unique_ptr<SDL_CameraID, decltype(camera_ids_deleter)>(
      SDL_GetCameras(&camera_count), camera_ids_deleter);
  if ((cameras_ids == nullptr) or (camera_count == 0)) {
    grape::syslog::Critical("No cameras found, or unable to enumerate cameras: {}", SDL_GetError());
    return false;
  }

  grape::syslog::Info("Found {} camera{}", camera_count, camera_count > 1 ? "s" : "");
  for (int i = 0; i < camera_count; ++i) {
    const auto id = cameras_ids.get()[i];
    printCameraSpecs(id);
  }

  grape::syslog::Info("Opening first camera");
  camera_.reset(SDL_OpenCamera(cameras_ids.get()[0], nullptr));
  if (camera_ == nullptr) {
    grape::syslog::Critical("Unable to open camera: {}", SDL_GetError());
    return false;
  }

  return true;
}

//-------------------------------------------------------------------------------------------------
void Camera::save() {
  save_snapshot_ = true;
}

//-------------------------------------------------------------------------------------------------
auto Camera::acquire() -> bool {
  auto* frame = SDL_AcquireCameraFrame(camera_.get(), nullptr /*timestamp*/);
  if (frame == nullptr) {
    return true;  // No frame available, but continue
  }

  stats_.compute();

  if (save_snapshot_) {
    save_snapshot_ = false;
    save(frame);
  }

  const auto frame_data = Frame{ .pixels = frame->pixels,
                                 .pitch = frame->pitch,
                                 .width = frame->w,
                                 .height = frame->h,
                                 .format = frame->format,
                                 .timestamp = std::chrono::system_clock::now() };

  if (frame_callback_) {
    frame_callback_(frame_data);
  }

  SDL_ReleaseCameraFrame(camera_.get(), frame);
  return true;
}

//-------------------------------------------------------------------------------------------------
void Camera::save(SDL_Surface* frame) {
  auto fname = std::format("snapshot_{:%FT%T}.bmp", std::chrono::system_clock::now());
  if (not SDL_SaveBMP(frame, fname.c_str())) {
    grape::syslog::Warn("Could not save snapshot");
  } else {
    grape::syslog::Info("Saved snapshot to {}", fname);
  }
}

//-------------------------------------------------------------------------------------------------
auto Camera::getCameraDrivers() -> std::vector<std::string> {
  const auto num_drivers = SDL_GetNumCameraDrivers();
  std::vector<std::string> drivers;
  for (int i = 0; i < num_drivers; ++i) {
    const char* driver_name = SDL_GetCameraDriver(i);
    drivers.emplace_back(driver_name);
  }
  return drivers;
}

//-------------------------------------------------------------------------------------------------
void Camera::printCameraSpecs(SDL_CameraID camera_id) {
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

//-------------------------------------------------------------------------------------------------
auto Display::init() -> bool {
  static constexpr auto DEFAULT_WIDTH = 640;
  static constexpr auto DEFAULT_HEIGHT = 480;

  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;

  if (not SDL_CreateWindowAndRenderer("Camera View", DEFAULT_WIDTH, DEFAULT_HEIGHT,
                                      SDL_WINDOW_RESIZABLE, &window, &renderer)) {
    grape::syslog::Critical("SDL_CreateWindowAndRenderer failed: {}", SDL_GetError());
    return false;
  }

  window_.reset(window);
  renderer_.reset(renderer);
  return true;
}

//-------------------------------------------------------------------------------------------------
void Display::render(const Frame& frame) {
  // Create or recreate texture if needed
  if (texture_ == nullptr || texture_->w != frame.width || texture_->h != frame.height ||
      texture_->format != frame.format) {
    texture_.reset(SDL_CreateTexture(renderer_.get(), frame.format, SDL_TEXTUREACCESS_STREAMING,
                                     frame.width, frame.height));
    if (texture_ == nullptr) {
      grape::syslog::Error("SDL_CreateTexture failed: {}", SDL_GetError());
      return;
    }
  }

  // Update texture with new frame data
  if (not SDL_UpdateTexture(texture_.get(), nullptr, frame.pixels, frame.pitch)) {
    grape::syslog::Error("SDL_UpdateTexture failed: {}", SDL_GetError());
  }

  // Setup background colour
  constexpr auto BGCOLOR = std::array<float, 4>{ 0.4F, 0.6F, 1.0F, SDL_ALPHA_OPAQUE_FLOAT };
  if (not SDL_SetRenderDrawColorFloat(renderer_.get(), BGCOLOR.at(0), BGCOLOR.at(1), BGCOLOR.at(2),
                                      BGCOLOR.at(3))) {
    grape::syslog::Warn("SDL_SetRenderDrawColorFloat failed: {}", SDL_GetError());
  }

  if (not SDL_RenderClear(renderer_.get())) {
    grape::syslog::Warn("SDL_RenderClear failed: {}", SDL_GetError());
  }

  int dw = 0;
  int dh = 0;
  SDL_GetRenderOutputSize(renderer_.get(), &dw, &dh);
  auto dst = SDL_FRect{ .x = 0, .y = 0, .w = static_cast<float>(dw), .h = static_cast<float>(dh) };

  // Calculate view dimensions maintaining aspect ratio
  const auto src_w = static_cast<float>(texture_->w);
  const auto src_h = static_cast<float>(texture_->h);
  const auto scale_w = dst.w / src_w;
  const auto scale_h = dst.h / src_h;
  const auto scale = std::min(scale_w, scale_h);
  const auto new_w = src_w * scale;
  const auto new_h = src_h * scale;
  dst.x = (dst.w - new_w) / 2;
  dst.y = (dst.h - new_h) / 2;
  dst.w = new_w;
  dst.h = new_h;

  if (not SDL_RenderTexture(renderer_.get(), texture_.get(), nullptr, &dst)) {
    grape::syslog::Warn("SDL_RenderTexture failed: {}", SDL_GetError());
  }

  if (not SDL_RenderPresent(renderer_.get())) {
    grape::syslog::Warn("SDL_RenderPresent failed: {}", SDL_GetError());
  }
}

//-------------------------------------------------------------------------------------------------
Application::Application() {
  display_ = std::make_unique<Display>();
  capture_ = std::make_unique<Camera>([this](const Frame& frame) { display_->render(frame); });
}

//-------------------------------------------------------------------------------------------------
auto Application::init() -> SDL_AppResult {
  if (not display_->init()) {
    return SDL_APP_FAILURE;
  }
  if (not capture_->init()) {
    return SDL_APP_FAILURE;
  }
  return SDL_APP_CONTINUE;
}

//-------------------------------------------------------------------------------------------------
auto Application::iterate() -> SDL_AppResult {
  if (not capture_->acquire()) {
    return SDL_APP_FAILURE;
  }
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
      capture_->save();
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
auto SDL_AppInit(void** appstate, int /*argc*/, char** /*argv*/) -> SDL_AppResult {
  setupLogging();

  if (not SDL_Init(SDL_INIT_VIDEO | SDL_INIT_CAMERA)) {
    grape::syslog::Critical("SDL_Init failed: {}", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  auto app = std::make_unique<Application>();
  const auto result = app->init();
  if (result != SDL_APP_CONTINUE) {
    return result;
  }

  *appstate = app.get();

  // Transfer ownership to static storage for automatic cleanup on exit
  static auto app_holder = std::move(app);

  return SDL_APP_CONTINUE;
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
