//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <chrono>
#include <thread>

#include "grape/app/app.h"
#include "grape/ipc/publisher.h"
#include "grape/log/syslog.h"

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {  // NOLINT(bugprone-exception-escape)
  // initialise application
  grape::app::init(argc, argv, "GRAPE publisher application example");

  // dummy data serializer
  const auto serialise = [](const std::string& msg) -> std::span<const std::byte> {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return { reinterpret_cast<const std::byte*>(msg.data()), msg.size() };
  };

  // create publisher
  static constexpr auto TOPIC = grape::ipc::Topic{ .name = "hello" };
  auto pub = grape::ipc::Publisher(TOPIC);

  // publish messages
  auto count = 0U;
  static constexpr auto LOOP_SLEEP = std::chrono::milliseconds(100);
  while (grape::app::ok()) {
    std::this_thread::sleep_for(LOOP_SLEEP);

    auto msg = std::format("Hello World {}", ++count);
    grape::syslog::Log(grape::log::Severity::Info, "{}", msg);
    pub.publish(serialise(msg));
  }

  return EXIT_SUCCESS;
}
