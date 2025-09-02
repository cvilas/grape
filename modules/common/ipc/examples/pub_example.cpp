//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <cstddef>
#include <print>
#include <thread>

#include "grape/exception.h"
#include "grape/ipc/publisher.h"
#include "grape/ipc/session.h"
#include "topic_example.h"

//=================================================================================================
// Demonstrates a basic IPC publisher. See sub_example.cpp for the corresponding subscriber.
auto main() -> int {
  try {
    grape::ipc::init(grape::ipc::Config{});

    const auto match_cb = [](const grape::ipc::Match& match) -> void {
      std::println("\n{} (entity: {})", toString(match.status), toString(match.remote_entity));
    };

    const auto topic = grape::ipc::ex::ExampleTopicAttributes{};
    auto publisher = grape::ipc::Publisher(topic, match_cb);

    auto counter = 0U;
    constexpr auto SLEEP_TIME = std::chrono::milliseconds(500);
    while (grape::ipc::ok()) {
      const auto message = std::string("Hello World ") + std::to_string(++counter);
      std::println("Sending message: '{}'", message);
      const auto status = publisher.publish(message);
      if (not status) {
        std::println("Publish failed: {}", toString(status.error()));
      }
      std::this_thread::sleep_for(SLEEP_TIME);
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
