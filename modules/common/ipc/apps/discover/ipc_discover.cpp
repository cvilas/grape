//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <chrono>
#include <print>
#include <thread>

#include "grape/conio/program_options.h"
#include "grape/exception.h"
#include "grape/ipc/discovery.h"
#include "grape/ipc/session.h"
#include "grape/utils/enums.h"

//=================================================================================================
// Discovers IPC endpoints and prints them to the console.
//
// Typical usage:
// ```code
// ipc_discover [--scope=Network|Host] [--interval=2.0]
// ```
//=================================================================================================

namespace {

//-------------------------------------------------------------------------------------------------
void printDiscovery() {
  std::println("\n---");
  const auto discovered_topics = grape::ipc::discover();
  if (discovered_topics.empty()) {
    std::println("(no topics discovered)");
    return;
  }

  for (const auto& [name, info] : discovered_topics) {
    std::println("\033[1mtopic: '{}'\033[0m", name);
    std::println("  publishers: {}", info.publishers.size());
    for (const auto& ep : info.publishers) {
      std::println("    {}", toString(ep));
    }
    std::println("  subscribers: {}", info.subscribers.size());
    for (const auto& ep : info.subscribers) {
      std::println("    {}", toString(ep));
    }
  }
}

}  // namespace

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    namespace ipc = grape::ipc;
    namespace conio = grape::conio;
    namespace enums = grape::enums;

    static constexpr auto DEFAULT_INTERVAL = 2.0;
    static constexpr auto DEFAULT_SCOPE = ipc::Config::Scope::Host;

    const auto args =
        conio::ProgramDescription("Discover and print all IPC endpoints")
            .declareOption<double>("interval", "refresh interval in seconds", DEFAULT_INTERVAL)
            .declareOption<ipc::Config::Scope>("scope", "scope [Network|Host]", DEFAULT_SCOPE)
            .parse(argc, argv);

    const auto interval = std::chrono::duration<double>(args.getOption<double>("interval"));
    const auto scope = args.getOption<ipc::Config::Scope>("scope");

    ipc::init(ipc::Config{ .scope = scope });

    std::println("Discovering IPC endpoints. scope={}, interval={}", enums::name(scope), interval);

    std::println("Press CTRL-C to quit");
    while (ipc::ok()) {
      std::this_thread::sleep_for(interval);
      printDiscovery();
    }

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
