//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/picam/pi_camera.h"

#include <atomic>
#include <mutex>

#include <SDL3/SDL_pixels.h>
#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/formats.h>
#include <libcamera/framebuffer_allocator.h>
#include <libcamera/libcamera.h>
#include <libcamera/request.h>
#include <libcamera/stream.h>
#include <sys/mman.h>

#include "grape/exception.h"
#include "grape/log/syslog.h"

namespace grape::camera {

namespace {
//-------------------------------------------------------------------------------------------------
/// Map libcamera pixel format to SDL3 pixel format
auto toSDLPixelFormat(const libcamera::PixelFormat& fmt) -> SDL_PixelFormat {
  switch (fmt) {
    case libcamera::formats::NV12:
      return SDL_PIXELFORMAT_NV12;
    case libcamera::formats::YUV420:
      return SDL_PIXELFORMAT_YV12;
    case libcamera::formats::YUYV:
      return SDL_PIXELFORMAT_YUY2;
    case libcamera::formats::MJPEG:
      return SDL_PIXELFORMAT_MJPG;
    case libcamera::formats::RGB888:
      return SDL_PIXELFORMAT_RGB24;
    default:
      syslog::Warn("Unmapped pixel format: {}, using UNKNOWN", fmt.toString());
      return SDL_PIXELFORMAT_UNKNOWN;
  }
}
}  // namespace

//-------------------------------------------------------------------------------------------------
struct PiCamera::Impl {
  PiCamera::Callback callback{ nullptr };

  // Core libcamera objects
  std::unique_ptr<libcamera::CameraManager> camera_manager;
  std::shared_ptr<libcamera::Camera> camera;
  std::unique_ptr<libcamera::CameraConfiguration> config;
  std::unique_ptr<libcamera::FrameBufferAllocator> allocator;
  std::vector<std::unique_ptr<libcamera::Request>> requests;

  libcamera::Stream* stream = nullptr;
  std::map<int, std::pair<void*, unsigned int>> mapped_buffers;

  std::atomic<libcamera::Request*> latest_request{ nullptr };

  std::atomic_bool camera_started{ false };

  // Setup methods
  void setupCamera(const std::string& name_hint);
  void configureStream(const ImageSize& image_size);
  void allocateBuffers();
  void createRequests();
  void startCapture();

  // Runtime methods
  void processRequest(libcamera::Request* request);
  void requestComplete(libcamera::Request* request);
  auto queueRequest(libcamera::Request* request) -> int;
};

//-------------------------------------------------------------------------------------------------
void PiCamera::Impl::setupCamera(const std::string& name_hint) {
  camera_manager = std::make_unique<libcamera::CameraManager>();
  if (camera_manager->start() != 0) {
    panic("Failed to start camera manager");
  }

  auto cameras = camera_manager->cameras();
  if (cameras.empty()) {
    panic("No cameras found");
  }
  syslog::Info("Found {} camera{}", cameras.size(), cameras.size() > 1 ? "s" : "");

  std::string camera_name;
  camera = cameras[0];
  for (const auto& cam : cameras) {
    const auto& props = cam->properties();
    const auto& model = props.get(libcamera::properties::Model);
    const auto& id = cam->id();
    syslog::Note("{}\t({})", (model ? *model : "NoName"), id);
    if (model && !name_hint.empty() && (model->find(name_hint) != std::string::npos)) {
      camera = cam;
      camera_name = *model;
    }
  }
  if (camera_name.empty()) {
    const auto& props = camera->properties();
    const auto& model = props.get(libcamera::properties::Model);
    camera_name = model ? *model : "Unknown";
  }
  syslog::Note("Opening camera: {} ({})", camera_name, camera->id());

  if (camera->acquire() != 0) {
    panic("Failed to acquire camera");
  }
}

//-------------------------------------------------------------------------------------------------
void PiCamera::Impl::configureStream(const ImageSize& image_size) {
  config = camera->generateConfiguration({ libcamera::StreamRole::Viewfinder });
  if ((not config) || config->empty()) {
    panic("Failed to generate camera configuration");
  }
  libcamera::StreamConfiguration& stream_config = config->at(0);

  syslog::Debug("Supported formats:");
  for (const auto& fmt : stream_config.formats().pixelformats()) {
    const auto& sizes = stream_config.formats().sizes(fmt);
    for (const auto& size : sizes) {
      syslog::Debug("  {}x{}, {}", size.width, size.height, fmt.toString());
    }
  }
  // specify desired formats in order of preference and let the camera select one
  // @note: NV12 seems to be broken on the pi
  const auto desired_formats = { libcamera::formats::MJPEG, libcamera::formats::YUYV };
  for (const auto& fmt : desired_formats) {
    const auto& sizes = stream_config.formats().sizes(fmt);
    if (not sizes.empty()) {
      stream_config.pixelFormat = fmt;
      // Find size closest to target resolution by minimizing total pixel difference
      auto best_size = sizes[0];
      auto best_score = std::numeric_limits<int>::max();
      for (const auto& size : sizes) {
        const auto score =
            std::abs(static_cast<int>(size.width) - static_cast<int>(image_size.width)) +
            std::abs(static_cast<int>(size.height) - static_cast<int>(image_size.height));
        if (score < best_score) {
          best_score = score;
          best_size = size;
        }
      }
      stream_config.size = best_size;
      syslog::Info("Closet match to target resolution({}x{}): {}x{}, {}", image_size.width,
                   image_size.height, best_size.width, best_size.height, fmt.toString());
      break;
    }
  }

  // Validate configuration
  const auto validation = config->validate();
  if (validation == libcamera::CameraConfiguration::Invalid) {
    panic("Invalid camera configuration");
  }
  if (validation == libcamera::CameraConfiguration::Adjusted) {
    syslog::Warn("Camera configuration was adjusted by libcamera");
  }

  // Apply configuration
  if (camera->configure(config.get()) != 0) {
    panic("Failed to configure camera");
  }

  // Log the actual configuration that was applied
  const auto& final_config = config->at(0);
  syslog::Note("Configured format: {}x{}, {}", final_config.size.width, final_config.size.height,
               final_config.pixelFormat.toString());

  stream = config->at(0).stream();
}

//-------------------------------------------------------------------------------------------------
void PiCamera::Impl::allocateBuffers() {
  allocator = std::make_unique<libcamera::FrameBufferAllocator>(camera);

  const int ret = allocator->allocate(stream);
  if (ret < 0) {
    panic("Failed to allocate buffers");
  }

  syslog::Info("Allocated {} buffers", ret);
}

//-------------------------------------------------------------------------------------------------
void PiCamera::Impl::createRequests() {
  const auto& stream_buffers = allocator->buffers(stream);

  for (const auto& buffer : stream_buffers) {
    auto request = camera->createRequest();
    if (not request) {
      panic("Failed to create request");
    }

    if (request->addBuffer(stream, buffer.get()) != 0) {
      panic("Failed to add buffer to request");
    }

    // Map buffer memory
    for (const auto& plane : buffer->planes()) {
      // NOLINTNEXTLINE(misc-const-correctness)
      void* const memory = mmap(nullptr, plane.length, PROT_READ, MAP_SHARED, plane.fd.get(), 0);
      if (memory == MAP_FAILED) {
        panic("Failed to map buffer memory");
      }
      mapped_buffers[plane.fd.get()] = std::make_pair(memory, plane.length);
    }

    requests.push_back(std::move(request));
  }
}

//-------------------------------------------------------------------------------------------------
void PiCamera::Impl::startCapture() {
  // Connect request completion handler
  camera->requestCompleted.connect(this, &PiCamera::Impl::requestComplete);

  // Start camera
  if (camera->start() != 0) {
    panic("Failed to start camera");
  }

  camera_started.store(true, std::memory_order_release);

  // Queue initial requests
  for (auto& request : requests) {
    if (queueRequest(request.get()) != 0) {
      panic("Failed to queue initial request");
    }
  }

  syslog::Note("Camera capture started");
}

//-------------------------------------------------------------------------------------------------
auto PiCamera::Impl::queueRequest(libcamera::Request* request) -> int {
  if (!camera_started.load(std::memory_order_acquire)) {
    return -1;
  }
  return camera->queueRequest(request);
}

//-------------------------------------------------------------------------------------------------
void PiCamera::Impl::requestComplete(libcamera::Request* request) {
  if (request->status() == libcamera::Request::RequestCancelled) {
    return;
  }
  processRequest(request);
}

//-------------------------------------------------------------------------------------------------
void PiCamera::Impl::processRequest(libcamera::Request* request) {
  // Store latest frame, discarding previous if not consumed
  auto* old = latest_request.exchange(request, std::memory_order_release);
  // If there was an unconsumed frame, requeue it immediately
  if (old != nullptr) {
    old->reuse(libcamera::Request::ReuseBuffers);
    queueRequest(old);
  }
}

//-------------------------------------------------------------------------------------------------
PiCamera::PiCamera(const Config& config, Callback&& image_callback)
  : impl_(std::make_unique<Impl>()) {
  impl_->callback = std::move(image_callback);
  impl_->setupCamera(config.camera_name_hint);
  impl_->configureStream(config.image_size);
  impl_->allocateBuffers();
  impl_->createRequests();
  impl_->startCapture();
}

//-------------------------------------------------------------------------------------------------
PiCamera::~PiCamera() {
  if (impl_->camera) {
    if (impl_->camera_started.exchange(false, std::memory_order_acq_rel)) {
      impl_->camera->stop();
    }
    impl_->camera->requestCompleted.disconnect(impl_.get(), &PiCamera::Impl::requestComplete);
    impl_->camera->release();
  }

  // Unmap buffers
  for (auto& [fd, pair] : impl_->mapped_buffers) {
    munmap(pair.first, pair.second);
  }
  impl_->mapped_buffers.clear();

  if (impl_->camera_manager) {
    impl_->camera_manager->stop();
  }
}

//-------------------------------------------------------------------------------------------------
void PiCamera::acquire() {
  // Get latest frame
  libcamera::Request* request = impl_->latest_request.exchange(nullptr, std::memory_order_acquire);
  if (request == nullptr) {
    return;  // No frame available
  }

  if (request->status() != libcamera::Request::RequestComplete) {
    request->reuse(libcamera::Request::ReuseBuffers);
    impl_->queueRequest(request);
    return;
  }

  // Get the buffer
  auto* const buffer = request->findBuffer(impl_->stream);
  if (buffer == nullptr) {
    syslog::Warn("No buffer found in request");
    request->reuse(libcamera::Request::ReuseBuffers);
    impl_->queueRequest(request);
    return;
  }

  const auto timestamp = WallClock::now();
  const auto& stream_config = impl_->config->at(0);

  // Map buffer data
  const auto& plane = buffer->planes()[0];
  const auto& meta = buffer->metadata().planes()[0];
  void* data = impl_->mapped_buffers[plane.fd.get()].first;
  const auto length = std::min(meta.bytesused, plane.length);

  // Map libcamera pixel format to SDL format
  auto pitch = static_cast<std::uint32_t>(stream_config.stride);
  const auto sdl_format = toSDLPixelFormat(stream_config.pixelFormat);
  if (sdl_format == SDL_PIXELFORMAT_MJPG) {
    pitch = length;
  }

  // Create ImageFrame with actual pixel format from camera
  const auto frame = camera::ImageFrame{
    //
    .header = { .timestamp = timestamp,
                .pitch = pitch,
                .width = static_cast<std::uint16_t>(stream_config.size.width),
                .height = static_cast<std::uint16_t>(stream_config.size.height),
                .format = sdl_format },
    .pixels = { static_cast<std::byte*>(data), length }
  };

  syslog::Debug("Image stride: {}, width: {}, height: {}", pitch, stream_config.size.width,
                stream_config.size.height);

  if (impl_->callback != nullptr) {
    impl_->callback(frame);
  }

  // Requeue the request for continuous capture
  request->reuse(libcamera::Request::ReuseBuffers);
  impl_->queueRequest(request);
}

}  // namespace grape::camera
