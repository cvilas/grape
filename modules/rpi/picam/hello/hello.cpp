// SPDX-License-Identifier: GPL-2.0-or-later */
// Copyright (C) 2020, Ideas on Board Oy.
//
// hello: First libcamera application
// Follows steps from the user guide: https://libcamera.org/guides/application-developer.html

#include <atomic>
#include <csignal>
#include <cstdlib>
#include <print>

#include <libcamera/libcamera.h>

namespace {
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
std::atomic_flag s_exit = false;
void onSignal(int /*signum*/) {
  s_exit.test_and_set();
  s_exit.notify_one();
}

std::shared_ptr<libcamera::Camera> camera;

// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

// --------------------------------------------------------------------
// Handle RequestComplete
//
// For each Camera::requestCompleted Signal emitted from the Camera the connected Slot is invoked.
//
// The Slot is invoked in the CameraManager's thread, hence one should avoid any heavy processing
// here. The processing of the request shall be re-directed to the application's thread instead, so
// as not to block the CameraManager's thread for large amount of time.
//
// The Slot receives the Request as a parameter.
void requestComplete(libcamera::Request* request) {
  if (request->status() == libcamera::Request::RequestCancelled) {
    return;
  }
  std::println("\nRequest completed: {}", request->toString());

  // When a request has completed, it is populated with a metadata control list that allows an
  // application to determine various properties of the completed request. This can include the
  // timestamp of the Sensor capture, or its gain and exposure values, or properties from the IPA
  // such as the state of the 3A algorithms.
  //
  // ControlValue types have a toString, so to examine each request, print all the metadata for
  // inspection. A custom application can parse each of these items and process them according to
  // its needs.
  const auto& request_metadata = request->metadata();
  for (const auto& ctrl : request_metadata) {
    const auto* id = libcamera::controls::controls.at(ctrl.first);
    const auto& value = ctrl.second;

    std::println("\t{}={}", id->name(), value.toString());
  }

  // Each buffer has its own FrameMetadata to describe its state, or the usage of each buffer. While
  // in our simple capture we only provide one buffer per request, a request can have a buffer for
  // each stream that is established when configuring the camera.
  //
  // This allows a viewfinder and a still image to be processed at the same time, or to allow
  // obtaining the RAW capture buffer from the sensor along with the image as processed by the ISP.
  const auto& buffers = request->buffers();
  for (auto buffer_pair : buffers) {
    auto* buffer = buffer_pair.second;
    const auto& metadata = buffer->metadata();

    std::print(" seq: {}  bytes used: ", metadata.sequence);
    auto plane_num = 0U;
    for (const auto& plane : metadata.planes()) {
      std::print("{}", plane.bytesused);
      if (++plane_num < metadata.planes().size()) {
        std::print("/");
      }
    }
    std::println("");

    // Image data can be accessed here, but the FrameBuffer must be mapped by the application
  }
  request->reuse(libcamera::Request::ReuseBuffers);
  camera->queueRequest(request);
}
}  // namespace

auto main() -> int {
  try {
    std::ignore = signal(SIGINT, onSignal);
    std::ignore = signal(SIGTERM, onSignal);
    // Start camera manager
    //
    // The Camera Manager is responsible for enumerating all the Camera in the system, by
    // associating Pipeline Handlers with media entities registered in the system. When the
    // CameraManager is no longer to be used, it should be deleted. There can only be a single
    // CameraManager constructed within any process space.
    auto cm = std::make_unique<libcamera::CameraManager>();
    cm->start();

    // enumerate cameras
    auto cameras = cm->cameras();
    if (cameras.empty()) {
      std::println("No cameras found.");
      cm->stop();
      return EXIT_FAILURE;
    }

    // List available cameras
    //
    // Applications are responsible for deciding how to name cameras, and present that information
    // to the users. Every camera has a unique identifier, though this string is not designed to be
    // friendly for a human reader.
    //
    // To support human consumable names, libcamera provides camera properties that allow an
    // application to determine a naming scheme based on its needs. See libcamera::properties
    //
    for (auto const& cam : cameras) {
      const auto& props = cam->properties();
      const auto& model = props.get(libcamera::properties::Model);
      std::println("{} ({})", (model ? *model : "NoName"), cam->id());
    }

    // Pick the first camera
    //
    // Camera are entities created by pipeline handlers, inspecting the entities registered in the
    // system and reported to applications by the CameraManager.
    //
    // In general terms, a Camera corresponds to a single image source available in the system, such
    // as an image sensor.
    //
    // Application lock usage of Camera by 'acquiring' them. Once done with it, application shall
    // similarly 'release' the Camera.
    //
    // Cameras can be obtained by their ID or their index
    const auto& camera_id = cameras[0]->id();
    camera = cm->get(camera_id);
    camera->acquire();

    /*
     * Stream
     *
     * Each Camera supports a variable number of Stream. A Stream is produced by processing data
     * produced by an image source, usually by an ISP.
     *
     *   +-------------------------------------------------------+
     *   | Camera                                                |
     *   |                +-----------+                          |
     *   | +--------+     |           |------> [  Main output  ] |
     *   | | Image  |     |           |                          |
     *   | |        |---->|    ISP    |------> [   Viewfinder  ] |
     *   | | Source |     |           |                          |
     *   | +--------+     |           |------> [ Still Capture ] |
     *   |                +-----------+                          |
     *   +-------------------------------------------------------+
     *
     * The number and capabilities of the Stream in a Camera are a platform dependent property, and
     * the pipeline handler implementation has the responsibility to correctly report them.
     */

    // Generate a default camera configuration for view finder role we want to use the camera for
    //
    // Camera configuration is tricky! It boils down to assigning resources of the system (such as
    // DMA engines, scalers, format converters) to the different image streams an application has
    // requested.
    //
    // Depending on the system characteristics, some combinations of sizes, formats and stream
    // usages might or might not be possible.
    //
    // A Camera produces a CameraConfigration based on a set of intended roles for each Stream the
    // application requires.
    auto config = camera->generateConfiguration({ libcamera::StreamRole::Viewfinder });

    // The CameraConfiguration contains a StreamConfiguration instance for each StreamRole requested
    // by the application, provided the Camera can support all of them.
    //
    // Each StreamConfiguration has default size and format, assigned by the Camera depending on the
    // Role the application has requested.
    auto& stream_config = config->at(0);
    std::println("Default viewfinder configuration is: {}", stream_config.toString());

    // Change and validate configuration
    //
    // Each StreamConfiguration parameter which is part of a CameraConfiguration can be
    // independently modified by the application.
    //
    // To validate the modified parameter, the CameraConfiguration should be validated -before- it
    // gets applied to the Camera. This will adjust the modified parameters to a valid configuration
    // which is as close as possible to the one requested.
    constexpr auto WIDTH = 640U;
    constexpr auto HEIGHT = 480U;
    stream_config.size.width = WIDTH;
    stream_config.size.height = HEIGHT;
    config->validate();
    std::println("Validated viewfinder configuration is: {}", stream_config.toString());

    // Apply the configuration
    camera->configure(config.get());

    // Allocate framebuffers
    //
    // Now that a camera has been configured, it knows all about its Streams sizes and formats. The
    // captured images need to be stored in framebuffers which can either be provided by the
    // application to the library, or allocated in the Camera and exposed to the application by
    // libcamera.
    //
    // An application may decide to allocate framebuffers from elsewhere, for example in memory
    // allocated by the display driver that will render the captured frames. The application will
    // provide them to libcamera by constructing FrameBuffer instances to capture images directly
    // into.
    //
    // Alternatively libcamera can help the application by exporting buffers allocated in the Camera
    // using a FrameBufferAllocator instance and referencing a configured Camera to determine the
    // appropriate buffer size and types to create.
    auto allocator = std::make_unique<libcamera::FrameBufferAllocator>(camera);
    for (auto& cfg : *config) {
      if (allocator->allocate(cfg.stream()) < 0) {
        std::println("Can't allocate buffers");
        return -ENOMEM;
      }
      auto num_allocated = allocator->buffers(cfg.stream()).size();
      std::println("Allocated {} buffers for stream", num_allocated);
    }

    // Create frame capture requests
    //
    // libcamera frames capture model is based on the 'Request' concept. For each frame a Request
    // has to be queued to the Camera.
    //
    // A Request refers to (at least one) Stream for which a Buffer that will be filled with image
    // data shall be added to the Request.
    //
    // A Request is associated with a list of Controls, which are tunable parameters (similar to
    // v4l2_controls) that have to be applied to the image.
    //
    // Once a request completes, all its buffers will contain image data that applications can
    // access and for each of them a list of metadata properties that reports the capture parameters
    // applied to the image.
    auto* stream = stream_config.stream();
    const auto& buffers = allocator->buffers(stream);

    std::vector<std::unique_ptr<libcamera::Request>> requests;
    for (const auto& buffer : buffers) {
      auto request = camera->createRequest();
      if (!request) {
        std::println("Can't create request");
        return -ENOMEM;
      }

      if (request->addBuffer(stream, buffer.get()) < 0) {
        std::println("Can't set buffer for request");
        return EXIT_FAILURE;
      }

      // Controls can be added to a request on a per frame basis.
      auto& controls = request->controls();
      constexpr auto BRIGHTNESS = 0.5;
      controls.set(libcamera::controls::Brightness, BRIGHTNESS);

      requests.push_back(std::move(request));
    }

    // Setup request completion handler
    //
    // libcamera uses a Signal & Slot based system to connect events to callback operations meant to
    // handle them, inspired by the QT graphic toolkit.
    //
    // Signals are events 'emitted' by a class instance. Slots are callbacks that can be 'connected'
    // to a Signal.
    //
    // A Camera exposes Signals, to report the completion of a Request and the completion of a
    // Buffer part of a Request to support partial Request completions.
    //
    // In order to receive the notification for request completions, applications shall connect a
    // Slot to the Camera 'requestCompleted' Signal before the camera is started.
    camera->requestCompleted.connect(requestComplete);

    // Start capture by queueing requests
    //
    // In order to capture frames the Camera has to be started and Request queued to it. Enough
    // Request to fill the Camera pipeline depth have to be queued before the Camera start
    // delivering frames.
    //
    // For each delivered frame, the Slot connected to the Camera::requestCompleted Signal is
    // called.
    camera->start();
    for (auto& request : requests) {
      camera->queueRequest(request.get());
    }

    // handle events
    std::println("press ctrl-c to exit");
    s_exit.wait(false);

    // Cleanup and exit
    //
    // Stop the Camera, release resources and stop the CameraManager. libcamera has now released all
    // resources it owned.
    camera->stop();
    allocator->free(stream);
    camera->release();
    camera.reset();
    cm->stop();

    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::puts(ex.what());
    return EXIT_FAILURE;
  }
}
