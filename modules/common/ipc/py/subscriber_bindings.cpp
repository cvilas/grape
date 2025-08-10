//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <pybind11/chrono.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "bindings.h"
#include "grape/ipc/raw_subscriber.h"

namespace grape::ipc::py {

void bindSubscriber(pybind11::module_& module) {
  // Bind SampleInfo struct
  pybind11::class_<SampleInfo>(module, "SampleInfo")
      .def_readonly("publish_time", &SampleInfo::publish_time, "The time the data was published")
      .def_readonly("publisher", &SampleInfo::publisher, "The publisher of the data");

  // Bind the Sample struct
  pybind11::class_<Sample>(module, "Sample")
      .def(
          "data",
          [](const Sample& sample) -> pybind11::bytes {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            return { reinterpret_cast<const char*>(sample.data.data()), sample.data.size() };
          },
          "The data received by the subscriber")
      .def_readonly("info", &Sample::info, "Meta information about the sample");

  // Bind the RawSubscriber class
  pybind11::class_<RawSubscriber>(module, "RawSubscriber")
      .def(pybind11::init<const std::string&, RawSubscriber::DataCallback, MatchCallback>(),
           pybind11::arg("topic"), pybind11::arg("data_cb"), pybind11::arg("match_cb") = nullptr,
           "Create a RawSubscriber with the specified topic, data callback, and optional match "
           "callback.")
      .def("get_publisher_count", &RawSubscriber::publisherCount,
           "Get the number of publishers currently matched to this subscriber.");
}

}  // namespace grape::ipc::py
