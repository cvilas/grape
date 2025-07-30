//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/camera/camera.h"

#include <SDL3/SDL_camera.h>

#include "grape/exception.h"
#include "grape/log/syslog.h"
#include "grape/utils/format_ranges.h"

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
  auto** specs = SDL_GetCameraSupportedFormats(camera_id, nullptr);
  if (specs == nullptr) {
    grape::syslog::Warn("Unable to get camera specs: {}", SDL_GetError());
    return;
  }
  grape::syslog::Info("  Supported specs:");
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  for (int i = 0; specs[i] != nullptr; ++i) {
    const auto* sp = specs[i];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    grape::syslog::Info("  {}x{}, {} FPS, {}", sp->width, sp->height,
                        sp->framerate_numerator / sp->framerate_denominator,
                        SDL_GetPixelFormatName(sp->format));
  }
  // NOLINTNEXTLINE(cppcoreguidelines-no-malloc,cppcoreguidelines-owning-memory,bugprone-multi-level-implicit-pointer-conversion)
  SDL_free(specs);
}

//-------------------------------------------------------------------------------------------------
auto calcPixelBufferSize(const SDL_Surface* surf) -> int {
  if (surf->format == SDL_PIXELFORMAT_NV12) {
    return (surf->w * surf->h * 3) / 2;
  }
  if (surf->format == SDL_PIXELFORMAT_MJPG) {
    return surf->pitch;
  }
  // TODO(Vilas): Support more formats here as required
  grape::panic<grape::Exception>(
      std::format("Unsupported format {}", SDL_GetPixelFormatName(surf->format)));
  return 0;
}

}  // namespace

namespace grape::camera {

struct Camera::Impl {
  std::unique_ptr<SDL_Camera, void (*)(SDL_Camera*)> camera{ nullptr, SDL_CloseCamera };
};

//-------------------------------------------------------------------------------------------------
Camera::Camera(Callback callback, const std::string& name_hint)
  : impl_(std::make_unique<Impl>()), callback_(std::move(callback)) {
  syslog::Info("Available camera drivers: {}", enumerateCameraDrivers());
  syslog::Info("Using camera driver: {}", SDL_GetCurrentCameraDriver());

  auto camera_count = 0;
  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,cppcoreguidelines-no-malloc)
  auto camera_ids_deleter = [](SDL_CameraID* ptr) { SDL_free(ptr); };
  auto cameras_ids = std::unique_ptr<SDL_CameraID, decltype(camera_ids_deleter)>(
      SDL_GetCameras(&camera_count), camera_ids_deleter);
  if ((cameras_ids == nullptr) or (camera_count == 0)) {
    panic<Exception>(std::format("No cameras enumerated: {}", SDL_GetError()));
  }

  auto chosen_camera_index = -1;
  syslog::Info("Found {} camera{}", camera_count, camera_count > 1 ? "s" : "");
  for (int i = 0; i < camera_count; ++i) {
    const auto id = cameras_ids.get()[i];
    const auto camera_name = std::string(SDL_GetCameraName(id));
    if (camera_name.find(name_hint) != std::string::npos) {
      chosen_camera_index = i;
    }
    syslog::Info("[{}] {}", i, camera_name);
    printCameraSpecs(id);
  }
  if (chosen_camera_index < 0) {
    syslog::Warn("No camera found matching '{}'. Will open default camera", name_hint);
    chosen_camera_index = 0;
  }

  syslog::Info("Opening camera number {}", chosen_camera_index);
  impl_->camera.reset(SDL_OpenCamera(cameras_ids.get()[chosen_camera_index], nullptr));
  if (impl_->camera == nullptr) {
    panic<Exception>(std::format("Unable to open camera: {}", SDL_GetError()));
  }
}

//-------------------------------------------------------------------------------------------------
Camera::~Camera() = default;

//-------------------------------------------------------------------------------------------------
void Camera::acquire() {
  auto timestamp_ns = std::uint64_t{};  // unused
  auto* sdl_frame = SDL_AcquireCameraFrame(impl_->camera.get(), &timestamp_ns);
  const auto now = std::chrono::system_clock::now();
  if (sdl_frame == nullptr) {
    return;  // No frame available, but continue
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

  callback_(frame);

  SDL_ReleaseCameraFrame(impl_->camera.get(), sdl_frame);
}

}  // namespace grape::camera
