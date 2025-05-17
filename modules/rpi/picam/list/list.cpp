//=================================================================================================
// Copyright (C) 2025 grape contributors
//=================================================================================================

#include <cstdlib>

#include <libcamera/libcamera.h>

#include "grape/log/syslog.h"

//=================================================================================================
// Lists libcamera supported image capture devices available on the host
auto main() -> int {
  try {
    auto log_config = grape::log::Config{ .threshold = grape::log::Severity::Note };
    grape::syslog::init(std::move(log_config));

    auto cm = std::make_unique<libcamera::CameraManager>();
    cm->start();

    auto cameras = cm->cameras();
    if (cameras.empty()) {
      grape::syslog::Critical("No cameras found.");
      cm->stop();
      return EXIT_FAILURE;
    }

    for (auto const& device : cameras) {
      const auto& props = device->properties();
      const auto& model = props.get(libcamera::properties::Model);
      const auto& id = device->id();
      auto camera = cm->get(id);
      grape::syslog::Note("{}\t({})", (model ? *model : "NoName"), id);
      const auto stream_configs =
          camera->generateConfiguration({ libcamera::StreamRole::Viewfinder });
      for (const auto& stream : *stream_configs) {
        grape::syslog::Note("\t{}", stream.toString());
      }
    }

    cm->stop();

    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::puts(ex.what());
    return EXIT_FAILURE;
  }
}
