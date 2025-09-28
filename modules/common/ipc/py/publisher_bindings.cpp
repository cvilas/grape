//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <nanobind/stl/function.h>
#include <nanobind/stl/string.h>

#include "bindings.h"
#include "grape/ipc/raw_publisher.h"

namespace grape::ipc::py {

void bindPublisher(nanobind::module_& module) {
  nanobind::class_<RawPublisher>(module, "RawPublisher")
      .def(nanobind::init<const std::string&, MatchCallback&&>(), nanobind::arg("topic"),
           nanobind::arg("match_cb") = nullptr,
           "Create a RawPublisher with the specified topic and optional match callback.")
      .def("get_subscriber_count", &RawPublisher::subscriberCount,
           "Get the number of subscribers currently matched to this publisher.")
      .def(
          "publish",
          [](const RawPublisher& self, const nanobind::bytes& data) -> bool {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            const auto* raw_data = reinterpret_cast<const std::byte*>(data.data());
            const std::size_t size = data.size();
            const std::span<const std::byte> bytes(raw_data, size);
            return self.publish(bytes).has_value();
          },
          nanobind::arg("data"), "Publish data on the topic specified at creation.");
}
}  // namespace grape::ipc::py
