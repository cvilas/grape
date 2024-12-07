//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <print>
#include <thread>

#include "grape/app/app.h"
#include "grape/conio/program_options.h"
#include "grape/exception.h"

namespace {

//-------------------------------------------------------------------------------------------------
std::atomic_flag s_exit = false;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

void onSignal(int /*signum*/) {
  s_exit.test_and_set();
  s_exit.notify_one();
}

}  // namespace

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    std::ignore = signal(SIGINT, onSignal);
    std::ignore = signal(SIGTERM, onSignal);

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

    static constexpr auto topic = grape::ipc::Topic{ .key = "hello" };
    auto pub = grape::app::createPublisher(topic);

    auto count = 0U;
    static constexpr auto LOOP_SLEEP = std::chrono::milliseconds(100);
    while (not s_exit.test()) {
      std::this_thread::sleep_for(LOOP_SLEEP);

      auto msg = std::format("Hello World {}", ++count);
      grape::app::syslog(grape::log::Severity::Info, "{}", msg);
      pub.publish({ msg.data(), msg.size() });
    }

    // cleanup before exit
    grape::app::cleanup();

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
