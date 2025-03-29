//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <cstdlib>
#include <print>

#include "grape/app/app.h"
#include "grape/conio/program_options.h"
#include "grape/exception.h"
#include "grape/log/syslog.h"

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
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

    static constexpr auto TOPIC = "hello";
    const auto deserialise = [](std::span<const std::byte> bytes) -> std::string {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      return { reinterpret_cast<const char*>(bytes.data()), bytes.size() };
    };
    const auto callback = [&deserialise](const grape::ipc::Sample& sample) {
      const auto msg = deserialise(sample.data);
      grape::syslog::Log(grape::log::Severity::Info, "{}", msg);
    };
    auto sub = grape::app::createSubscriber(TOPIC, callback);

    std::println("Press ctrl-c to exit");
    grape::app::waitForExit();

    // cleanup before exit
    grape::app::cleanup();

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
