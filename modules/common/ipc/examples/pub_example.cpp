//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <cstddef>
#include <print>
#include <thread>

#include "grape/exception.h"
#include "grape/ipc/publisher.h"
#include "grape/ipc/session.h"

//=================================================================================================
// Demonstrates a basic IPC publisher. See sub_example.cpp for the corresponding subscriber.
auto main() -> int {
  try {
    grape::ipc::init(grape::ipc::Config{});
    const auto topic = grape::ipc::Topic{ .name = "hello_world" };

    const auto match_cb = [](const grape::ipc::Match& match) -> void {
      if (match.status == grape::ipc::Match::Status::Matched) {
        std::println("\nMatched");
      } else if (match.status == grape::ipc::Match::Status::Unmatched) {
        std::println("\nUnmatched");
      }
    };

    auto publisher = grape::ipc::Publisher(topic, match_cb);

    const auto to_bytes = [](const std::string& msg) -> std::span<const std::byte> {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      return { reinterpret_cast<const std::byte*>(msg.data()), msg.size() };
    };
    auto counter = 0U;
    constexpr auto SLEEP_TIME = std::chrono::milliseconds(500);
    while (grape::ipc::ok()) {
      const auto message = std::string("Hello World ") + std::to_string(++counter);
      std::println("Sending message: '{}'", message);
      publisher.publish(to_bytes(message));
      std::this_thread::sleep_for(SLEEP_TIME);
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
