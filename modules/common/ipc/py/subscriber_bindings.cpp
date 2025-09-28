//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <nanobind/stl/chrono.h>
#include <nanobind/stl/function.h>
#include <nanobind/stl/string.h>

#include "bindings.h"
#include "grape/ipc/raw_subscriber.h"

namespace grape::ipc::py {

void bindSubscriber(nanobind::module_& module) {
  // Bind SampleInfo struct
  nanobind::class_<SampleInfo>(module, "SampleInfo")
      .def_ro("publish_time", &SampleInfo::publish_time, "The time the data was published")
      .def_ro("publisher", &SampleInfo::publisher, "The publisher of the data");

  // Bind the Sample struct
  nanobind::class_<Sample>(module, "Sample")
      .def(
          "data",
          [](const Sample& sample) -> nanobind::bytes {
            return nanobind::bytes(sample.data.data(), sample.data.size());
          },
          "The data received by the subscriber")
      .def_ro("info", &Sample::info, "Meta information about the sample");

  // Bind the RawSubscriber class
  nanobind::class_<RawSubscriber>(module, "RawSubscriber")
      .def(nanobind::init<const std::string&, RawSubscriber::DataCallback, MatchCallback>(),
           nanobind::arg("topic"), nanobind::arg("data_cb"), nanobind::arg("match_cb") = nullptr,
           "Create a RawSubscriber with the specified topic, data callback, and optional match "
           "callback.")
      .def("get_publisher_count", &RawSubscriber::publisherCount,
           "Get the number of publishers currently matched to this subscriber.");
}

}  // namespace grape::ipc::py
