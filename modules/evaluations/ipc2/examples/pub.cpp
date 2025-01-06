//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <cstddef>
#include <exception>
#include <print>
#include <thread>

#include "grape/ipc2/session.h"

//=================================================================================================
// Demonstrates a basic IPC publisher. See sub_example.cpp for the corresponding subscriber.
auto main(int argc, char* argv[]) -> int {
  (void)argc;
  (void)argv;
  try {
    const auto config = grape::ipc2::Session::Config{};
    auto session = grape::ipc2::Session(config);
    const auto topic = grape::ipc2::Topic{ .name = "hello_world" };

    const auto match_cb = [](const grape::ipc2::Match& match) {
      if (match.status == grape::ipc2::Match::Status::Matched) {
        std::println("\nMatched");
      } else if (match.status == grape::ipc2::Match::Status::Unmatched) {
        std::println("\nUnmatched");
      }
    };

    auto publisher = session.createPublisher(topic, match_cb);

    const auto to_bytes = [](const std::string& msg) -> std::span<const std::byte> {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      return { reinterpret_cast<const std::byte*>(msg.data()), msg.size() };
    };
    auto counter = 0U;
    constexpr auto SLEEP_TIME = std::chrono::milliseconds(500);
    while (session.ok()) {
      const auto message = std::string("Hello World ") + std::to_string(++counter);
      std::println("Sending message: '{}'", message);
      publisher.publish(to_bytes(message));
      std::this_thread::sleep_for(SLEEP_TIME);
    }
    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}
