//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <chrono>
#include <cstdlib>
#include <print>
#include <thread>

#include "grape/app/app.h"
#include "grape/conio/program_options.h"
#include "grape/exception.h"

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    // Parse command line arguments
    const auto maybe_args =
        grape::conio::ProgramDescription("GRAPE publisher application example")         //
            .declareOption<std::string>("config", "Top level configuration file path")  //
            .parse(argc, argv);
    if (not maybe_args.has_value()) {
      grape::panic<grape::Exception>(toString(maybe_args.error()));
    }
    const auto& args = maybe_args.value();

    // Read configuration file path
    const auto config = args.getOptionOrThrow<std::string>("config");

    // Initialise application
    grape::app::init({ config });

    static constexpr auto TOPIC = grape::ipc::Topic{ .name = "hello" };
    auto pub = grape::app::createPublisher(TOPIC);

    const auto to_bytes = [](const std::string& msg) -> std::span<const std::byte> {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      return { reinterpret_cast<const std::byte*>(msg.data()), msg.size() };
    };

    auto count = 0U;
    static constexpr auto LOOP_SLEEP = std::chrono::milliseconds(100);
    while (grape::app::ok()) {
      std::this_thread::sleep_for(LOOP_SLEEP);

      auto msg = std::format("Hello World {}", ++count);
      grape::app::syslog(grape::log::Severity::Info, "{}", msg);
      pub.publish(to_bytes(msg));
    }

    // cleanup before exit
    grape::app::cleanup();

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
