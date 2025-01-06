//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <exception>
#include <print>
#include <thread>

#include "grape/ipc2/session.h"

//=================================================================================================
// Demonstrates a basic IPC subscriber. See pub_example.cpp for the corresponding publisher.
auto main(int argc, char* argv[]) -> int {
  (void)argc;
  (void)argv;
  try {
    const auto config = grape::ipc2::Session::Config{};
    auto session = grape::ipc2::Session(config);
    const auto topic = grape::ipc2::Topic{ .name = "hello_world" };
    const auto from_bytes = [](std::span<const std::byte> bytes) -> std::string {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      return { reinterpret_cast<const char*>(bytes.data()), bytes.size() };
    };

    const auto data_cb = [&from_bytes](const grape::ipc2::Sample& sample) {
      std::println("Received message: '{}'", from_bytes(sample.data));
    };

    const auto match_cb = [](const grape::ipc2::Match& match) {
      if (match.status == grape::ipc2::Match::Status::Matched) {
        std::println("\nMatched");
      } else if (match.status == grape::ipc2::Match::Status::Unmatched) {
        std::println("\nUnmatched");
      }
    };

    auto subscriber = session.createSubscriber(topic.name, data_cb, match_cb);

    constexpr auto SLEEP_TIME = std::chrono::milliseconds(500);
    while (session.ok()) {
      std::this_thread::sleep_for(SLEEP_TIME);
    }
    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}
