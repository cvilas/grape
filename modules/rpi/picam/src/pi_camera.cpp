//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/picam/pi_camera.h"

#include <libcamera/libcamera.h>

#include "grape/exception.h"
#include "grape/log/syslog.h"

namespace grape::picam {

struct PiCamera::Impl {
  // Core libcamera objects
  std::unique_ptr<libcamera::CameraManager> camera_manager;
  std::shared_ptr<libcamera::Camera> camera;
  std::unique_ptr<libcamera::CameraConfiguration> config;
  std::unique_ptr<libcamera::FrameBufferAllocator> allocator;
  std::vector<std::unique_ptr<libcamera::Request>> requests;

  // Stream and buffer management
  libcamera::Stream* stream = nullptr;
  std::vector<std::unique_ptr<libcamera::FrameBuffer>> buffers;

  // Frame processing
  void processRequest(libcamera::Request* request);
  void setupCamera(const std::string& name_hint);
  void configureStream();
  void allocateBuffers();
  void createRequests();

  PiCamera::Callback* callback_ptr = nullptr;
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
    if (model && (model->find(name_hint) != std::string::npos)) {
      camera = cam;
      camera_name = *model;
    }
  }
  syslog::Note("Opening camera: {} ({})", camera_name, camera->id());

  if (camera->acquire() != 0) {
    panic(std::format("Failed to acquire camera"));
  }
}

//-------------------------------------------------------------------------------------------------
void PiCamera::Impl::configureStream() {
  // Generate camera configuration
  config = camera->generateConfiguration({ libcamera::StreamRole::Viewfinder });
  if (!config || config->empty()) {
    panic("Failed to generate camera configuration");
  }

  // Get the stream configuration
  libcamera::StreamConfiguration& stream_config = config->at(0);

  // Log supported formats
  syslog::Info("Supported formats:");
  for (const auto& fmt : stream_config.formats().pixelformats()) {
    const auto& sizes = stream_config.formats().sizes(fmt);
    for (const auto& size : sizes) {
      syslog::Info("  {}x{}, {}", size.width, size.height, fmt.toString());
    }
  }

  // Choose best format (prefer higher resolution)
  auto best_format = stream_config.formats().pixelformats()[0];
  auto best_size = libcamera::Size(0, 0);

  for (const auto& fmt : stream_config.formats().pixelformats()) {
    const auto& sizes = stream_config.formats().sizes(fmt);
    for (const auto& size : sizes) {
      if (size.width * size.height > best_size.width * best_size.height) {
        best_format = fmt;
        best_size = size;
      }
    }
  }

  // Configure the stream
  stream_config.pixelFormat = best_format;
  stream_config.size = best_size;

  syslog::Info("Selected format: {}x{}, {}", best_size.width, best_size.height,
               best_format.toString());

  // Validate configuration
  const auto validation = config->validate();
  if (validation == libcamera::CameraConfiguration::Invalid) {
    panic("Invalid camera configuration");
  }

  // Apply configuration
  if (camera->configure(config.get()) != 0) {
    panic(std::format("Failed to configure camera"));
  }

  stream = config->at(0).stream();
}

//-------------------------------------------------------------------------------------------------
void PiCamera::Impl::allocateBuffers() {
  allocator = std::make_unique<libcamera::FrameBufferAllocator>(camera);

  int ret = allocator->allocate(stream);
  if (ret < 0) {
    panic("Failed to allocate buffers");
  }

  syslog::Info("Allocated {} buffers", ret);

  // Get the allocated buffers
  const auto& stream_buffers = allocator->buffers(stream);
  buffers.reserve(stream_buffers.size());

  for (const auto& buffer : stream_buffers) {
    buffers.push_back(std::unique_ptr<libcamera::FrameBuffer>(buffer.get()));
  }
}

//-------------------------------------------------------------------------------------------------
void PiCamera::Impl::createRequests() {
  for (auto& buffer : buffers) {
    auto request = camera->createRequest();
    if (!request) {
      panic("Failed to create request");
    }

    if (request->addBuffer(stream, buffer.get()) != 0) {
      panic("Failed to add buffer to request");
    }

    requests.push_back(std::move(request));
  }
}

//-------------------------------------------------------------------------------------------------
void PiCamera::Impl::processRequest(libcamera::Request* request) {
  if (request->status() == libcamera::Request::RequestComplete) {
    // Get the buffer
    auto* const buffer = request->findBuffer(stream);
    if (buffer == nullptr) {
      syslog::Warn("No buffer found in request");
      return;
    }

    // Map buffer data (implementation depends on buffer type)
    // This is simplified - actual implementation would need proper buffer mapping
    // const auto& metadata = request->metadata();
    const auto timestamp = WallClock::now();  // or extract from metadata

    // Get stream configuration for frame info
    const auto& stream_config = config->at(0);

    // Create ImageFrame
    // Note: This is simplified - you'd need to properly map the buffer memory
    // and handle different pixel formats
    const auto frame = camera::ImageFrame{
      .header = { .pitch = stream_config.stride,
                  .width = static_cast<std::uint32_t>(stream_config.size.width),
                  .height = static_cast<std::uint32_t>(stream_config.size.height),
                  .format = static_cast<std::uint32_t>(
                      stream_config.pixelFormat.fourcc()),  // Convert libcamera
                                                            // format
                  .timestamp = timestamp },
      .pixels = { /* mapped buffer data */ }
    };

    // Invoke callback
    if (callback_ptr != nullptr) {
      (*callback_ptr)(frame);
    }
  }

  // Requeue the request for continuous capture
  camera->queueRequest(request);
}

//-------------------------------------------------------------------------------------------------
PiCamera::PiCamera(Callback&& callback, const std::string& name_hint)
  : impl_(std::make_unique<Impl>()), callback_(std::move(callback)) {
  impl_->callback_ptr = &callback_;

  // Setup libcamera pipeline
  impl_->setupCamera(name_hint);
  impl_->configureStream();
  impl_->allocateBuffers();
  impl_->createRequests();

  // Setup request completion handler
  impl_->camera->requestCompleted.connect(impl_.get(), &PiCamera::Impl::processRequest);

  // Start camera
  if (impl_->camera->start() != 0) {
    panic(std::format("Failed to start camera"));
  }

  // Queue initial requests
  for (auto& request : impl_->requests) {
    impl_->camera->queueRequest(request.get());
  }
}

//-------------------------------------------------------------------------------------------------
PiCamera::~PiCamera() {
  if (impl_->camera) {
    impl_->camera->stop();
    impl_->camera->release();
  }
  if (impl_->camera_manager) {
    impl_->camera_manager->stop();
  }
}
}  // namespace grape::picam
