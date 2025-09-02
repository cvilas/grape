//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <cstddef>
#include <print>
#include <thread>

#include "grape/exception.h"
#include "grape/ipc/raw_publisher.h"
#include "grape/ipc/session.h"

//=================================================================================================
// Demonstrates a basic IPC publisher. See raw_sub_example.cpp for the corresponding subscriber.
auto main() -> int {
  try {
    grape::ipc::init(grape::ipc::Config{});
    const auto* const topic = "hello_world";

    const auto match_cb = [](const grape::ipc::Match& match) -> void {
      std::println("\n{} (entity: {})", toString(match.status), toString(match.remote_entity));
    };

    auto publisher = grape::ipc::RawPublisher(topic, match_cb);

    const auto to_bytes = [](const std::string& msg) -> std::span<const std::byte> {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      return { reinterpret_cast<const std::byte*>(msg.data()), msg.size() };
    };
    auto counter = 0U;
    constexpr auto SLEEP_TIME = std::chrono::milliseconds(500);
    while (grape::ipc::ok()) {
      const auto message = std::string("Hello World ") + std::to_string(++counter);
      std::println("Sending message: '{}'", message);
      const auto result = publisher.publish(to_bytes(message));
      if (not result) {
        std::println("Publish failed: {}", toString(result.error()));
      }
      std::this_thread::sleep_for(SLEEP_TIME);
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
