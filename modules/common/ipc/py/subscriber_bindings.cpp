//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <pybind11/chrono.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "bindings.h"
#include "grape/ipc/subscriber.h"

namespace grape::ipc::py {

void bindSubscriber(pybind11::module_& module) {
  // Bind the Sample struct
  pybind11::class_<Sample>(module, "Sample")
      .def_property_readonly(
          "data",
          [](const Sample& sample) -> pybind11::bytes {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            return { reinterpret_cast<const char*>(sample.data.data()), sample.data.size() };
          },
          "The data received by the subscriber")
      .def_readonly("publish_time", &Sample::publish_time, "The time the data was published");

  // Bind the Subscriber class
  pybind11::class_<Subscriber>(module, "Subscriber")
      .def(pybind11::init<const std::string&, Subscriber::DataCallback, MatchCallback>(),
           pybind11::arg("topic"), pybind11::arg("data_cb"), pybind11::arg("match_cb") = nullptr,
           "Create a Subscriber with the specified topic, data callback, and optional match "
           "callback.")
      .def("get_publisher_count", &Subscriber::publisherCount,
           "Get the number of publishers currently matched to this subscriber.");
}

}  // namespace grape::ipc::py
