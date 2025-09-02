//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <print>
#include <thread>

#include "grape/exception.h"
#include "grape/ipc/session.h"
#include "grape/ipc/subscriber.h"
#include "topic_example.h"

//=================================================================================================
// Demonstrates a basic IPC subscriber. See pub_example.cpp for the corresponding publisher.
auto main() -> int {
  try {
    grape::ipc::init(grape::ipc::Config{});

    const auto data_cb = [](const std::expected<std::string, grape::ipc::Error>& data,
                            const grape::ipc::SampleInfo& info) -> void {
      if (data) {
        std::println("Received message: '{}' (from {})", *data, toString(info.publisher));
      } else {
        std::println("Error: {} (data from {})", toString(data.error()), toString(info.publisher));
      }
    };

    const auto match_cb = [](const grape::ipc::Match& match) -> void {
      std::println("\n{} (entity: {})", toString(match.status), toString(match.remote_entity));
    };

    const auto topic = grape::ipc::ex::ExampleTopicAttributes{};
    auto subscriber = grape::ipc::Subscriber(topic, data_cb, match_cb);

    constexpr auto SLEEP_TIME = std::chrono::milliseconds(500);
    while (grape::ipc::ok()) {
      std::this_thread::sleep_for(SLEEP_TIME);
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
