//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <csignal>
#include <cstdlib>
#include <print>

#include "grape/app/app.h"
#include "grape/conio/program_options.h"
#include "grape/exception.h"
#include "grape/log/severity.h"

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
        grape::conio::ProgramDescription("GRAPE subscriber application example")        //
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

    static constexpr auto topic = "hello";
    const auto callback = [](std::span<const std::byte> bytes) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      const auto msg = std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size_bytes());
      grape::app::syslog(grape::log::Severity::Info, "{}", msg);
    };
    auto sub = grape::app::createSubscriber(topic, callback);

    std::println("Press ctrl-c to exit");
    s_exit.wait(false);

    // cleanup before exit
    grape::app::cleanup();

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
