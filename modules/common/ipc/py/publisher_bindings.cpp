//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "bindings.h"
#include "grape/ipc/raw_publisher.h"

namespace grape::ipc::py {

void bindPublisher(pybind11::module_& module) {
  pybind11::class_<RawPublisher>(module, "RawPublisher")
      .def(pybind11::init<const std::string&, MatchCallback&&>(), pybind11::arg("topic"),
           pybind11::arg("match_cb") = nullptr,
           "Create a RawPublisher with the specified topic and optional match callback.")
      .def("get_subscriber_count", &RawPublisher::subscriberCount,
           "Get the number of subscribers currently matched to this publisher.")
      .def(
          "publish",
          [](const RawPublisher& self, const pybind11::bytes& data) -> bool {
            const std::string& data_str = data.cast<std::string>();
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            const auto* raw_data = reinterpret_cast<const std::byte*>(data_str.data());
            const std::size_t size = data_str.size();
            const std::span<const std::byte> bytes(raw_data, size);
            return self.publish(bytes).has_value();
          },
          pybind11::arg("data"), "Publish data on the topic specified at creation.");
}
}  // namespace grape::ipc::py
