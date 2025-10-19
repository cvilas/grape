//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/camera/camera.h"

#include <SDL3/SDL_camera.h>

#include "grape/exception.h"
#include "grape/log/syslog.h"
#include "grape/utils/format_ranges.h"
#include "helpers.h"

namespace {
//-------------------------------------------------------------------------------------------------
auto enumerateCameraDrivers() -> std::vector<std::string> {
  const auto num_drivers = SDL_GetNumCameraDrivers();
  std::vector<std::string> drivers;
  for (int i = 0; i < num_drivers; ++i) {
    const char* driver_name = SDL_GetCameraDriver(i);
    drivers.emplace_back(driver_name);
  }
  return drivers;
}

//-------------------------------------------------------------------------------------------------
void printCameraSpecs(SDL_CameraID camera_id) {
  int count = 0;
  auto** specs = SDL_GetCameraSupportedFormats(camera_id, &count);
  if (specs == nullptr) {
    grape::syslog::Warn("Unable to get camera specs: {}", SDL_GetError());
    return;
  }
  grape::syslog::Info("  Supported specs:");
  for (const auto* sp : std::span{ specs, static_cast<std::size_t>(count) }) {
    grape::syslog::Info("  {}x{}, {} FPS, {}", sp->width, sp->height,
                        sp->framerate_numerator / sp->framerate_denominator,
                        SDL_GetPixelFormatName(sp->format));
  }
  // NOLINTNEXTLINE(cppcoreguidelines-no-malloc,cppcoreguidelines-owning-memory,bugprone-multi-level-implicit-pointer-conversion)
  SDL_free(specs);
}

}  // namespace

namespace grape::camera {

struct Camera::Impl {
  void openCamera(SDL_CameraID id);
  std::unique_ptr<SDL_Camera, void (*)(SDL_Camera*)> camera{ nullptr, SDL_CloseCamera };
};

//-------------------------------------------------------------------------------------------------
void Camera::Impl::openCamera(SDL_CameraID id) {
  int count = 0;
  auto** specs = SDL_GetCameraSupportedFormats(id, &count);
  if (specs == nullptr) {
    grape::syslog::Warn("Unable to get camera specs: {}", SDL_GetError());
    return;
  }
  const auto specs_view = std::span{ specs, static_cast<std::size_t>(count) };
  auto best_score = 0;
  auto best_spec = *specs_view[0];
  for (const auto* sp : specs_view) {
    const auto fps = sp->framerate_numerator / sp->framerate_denominator;
    const auto score = sp->width * fps;  // choose the highest horizontal resolution and frames/sec
    if (score > best_score) {
      best_spec = *sp;
      best_score = score;
    }
  }
  // NOLINTNEXTLINE(cppcoreguidelines-no-malloc,cppcoreguidelines-owning-memory,bugprone-multi-level-implicit-pointer-conversion)
  SDL_free(specs);

  grape::syslog::Info("Best spec found: {}x{}, {} FPS, {}", best_spec.width, best_spec.height,
                      best_spec.framerate_numerator / best_spec.framerate_denominator,
                      SDL_GetPixelFormatName(best_spec.format));
  camera.reset(SDL_OpenCamera(id, &best_spec));
  if (camera == nullptr) {
    panic(std::format("Unable to open camera: {}", SDL_GetError()));
  }
}

//-------------------------------------------------------------------------------------------------
Camera::Camera(Callback&& callback, const std::string& name_hint)
  : impl_(std::make_unique<Impl>()), callback_(std::move(callback)) {
  syslog::Info("Available camera drivers: {}", enumerateCameraDrivers());
  syslog::Info("Using camera driver: {}", SDL_GetCurrentCameraDriver());

  auto camera_count = 0;
  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,cppcoreguidelines-no-malloc)
  auto camera_ids_deleter = [](SDL_CameraID* ptr) { SDL_free(ptr); };
  auto cameras_ids = std::unique_ptr<SDL_CameraID, decltype(camera_ids_deleter)>(
      SDL_GetCameras(&camera_count), camera_ids_deleter);
  if ((cameras_ids == nullptr) or (camera_count == 0)) {
    panic(std::format("No cameras enumerated: {}", SDL_GetError()));
  }

  auto chosen_camera_index = std::numeric_limits<std::size_t>::max();
  auto chosen_camera_name = std::string{};
  syslog::Info("Found {} camera{}", camera_count, camera_count > 1 ? "s" : "");
  for (auto i = 0U; std::cmp_less(i, camera_count); ++i) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const auto id = cameras_ids.get()[i];
    const auto camera_name = std::string(SDL_GetCameraName(id));
    syslog::Info("[{}] {}", i, camera_name);
    printCameraSpecs(id);
    if (camera_name.find(name_hint) != std::string::npos) {
      chosen_camera_index = i;
      chosen_camera_name = camera_name;
    }
  }
  if (chosen_camera_index == std::numeric_limits<std::size_t>::max()) {
    syslog::Warn("No camera found matching '{}'. Will open default camera", name_hint);
    chosen_camera_index = 0U;
  }

  syslog::Note("Opening camera [{}] {}", chosen_camera_index, chosen_camera_name);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  impl_->openCamera(cameras_ids.get()[chosen_camera_index]);
}

//-------------------------------------------------------------------------------------------------
Camera::~Camera() = default;

//-------------------------------------------------------------------------------------------------
void Camera::acquire() {
  auto timestamp_ns = std::uint64_t{};  // unused
  auto* sdl_frame = SDL_AcquireCameraFrame(impl_->camera.get(), &timestamp_ns);
  const auto now = WallClock::now();
  if (sdl_frame == nullptr) {
    return;  // No frame available, but continue
  }

  static bool call_once = true;
  if (call_once) {
    SDL_CameraSpec spec;
    if (SDL_GetCameraFormat(impl_->camera.get(), &spec)) {
      syslog::Note("Capturing at {}x{}, {} FPS, {}", spec.width, spec.height,
                   spec.framerate_numerator / spec.framerate_denominator,
                   SDL_GetPixelFormatName(spec.format));
    }
    call_once = false;
  }

  static const auto data_size = calcPixelBufferSize(sdl_frame);
  const auto frame = grape::camera::ImageFrame{
    .header = { .pitch = static_cast<std::uint32_t>(sdl_frame->pitch),
                .width = static_cast<std::uint32_t>(sdl_frame->w),
                .height = static_cast<std::uint32_t>(sdl_frame->h),
                .format = static_cast<std::uint32_t>(sdl_frame->format),
                .timestamp = now },
    .pixels = { static_cast<std::byte*>(sdl_frame->pixels), static_cast<std::size_t>(data_size) }
  };

  syslog::Debug("stride: {}, width: {}, height: {}", sdl_frame->pitch, sdl_frame->w, sdl_frame->h);

  callback_(frame);

  SDL_ReleaseCameraFrame(impl_->camera.get(), sdl_frame);
}

}  // namespace grape::camera
