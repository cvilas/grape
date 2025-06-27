//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <print>
#include <thread>

#include "grape/exception.h"
#include "grape/ipc/session.h"
#include "grape/ipc/subscriber.h"

//=================================================================================================
// Demonstrates a basic IPC subscriber. See pub_example.cpp for the corresponding publisher.
auto main() -> int {
  try {
    grape::ipc::init(grape::ipc::Config{});
    const auto* const topic = "hello_world";
    const auto from_bytes = [](std::span<const std::byte> bytes) -> std::string {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      return { reinterpret_cast<const char*>(bytes.data()), bytes.size() };
    };

    const auto data_cb = [&from_bytes](const grape::ipc::Sample& sample) -> void {
      std::println("Received message: '{}' (from {})", from_bytes(sample.data),
                   toString(sample.publisher));
    };

    const auto match_cb = [](const grape::ipc::Match& match) -> void {
      std::println("\n{} (entity: {})", toString(match.status), toString(match.remote_entity));
    };

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
