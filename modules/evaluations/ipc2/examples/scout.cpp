//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <atomic>

#include "grape/conio/program_options.h"
#include "zenoh_utils.h"

//=================================================================================================
// The process of discovering Zenoh applications is called scouting. For a discussion of how
// scouting works in a deployment consisting of peers, clients and routers, see
// https://zenoh.io/docs/getting-started/deployment/
//
// Derived from:
// https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/universal/z_scout.cxx
//=================================================================================================

namespace {

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toString(const zenoh::WhatAmI& me) -> std::string_view {
  switch (me) {
    case zenoh::WhatAmI::Z_WHATAMI_ROUTER:
      return "Router";
    case zenoh::WhatAmI::Z_WHATAMI_PEER:
      return "Peer";
    case zenoh::WhatAmI::Z_WHATAMI_CLIENT:
      return "Client";
  }
  return "";
}

}  // namespace

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    const auto maybe_args =
        grape::conio::ProgramDescription("Discovers Zenoh applications").parse(argc, argv);
    if (not maybe_args) {
      grape::panic<grape::Exception>(toString(maybe_args.error()));
    }

    std::atomic_flag done_flag = ATOMIC_FLAG_INIT;

    const auto on_hello = [](const zenoh::Hello& hello) {
      std::println("Hello from pid: {}, WhatAmI: {}, locators: {}",
                   grape::ipc2::ex::toString(hello.get_id()), toString(hello.get_whatami()),
                   hello.get_locators());
    };

    const auto on_good_bye = [&done_flag]() {
      done_flag.test_and_set();
      done_flag.notify_one();
    };

    std::println("Scouting..");
    auto config = zenoh::Config::create_default();
    static constexpr auto SCOUT_DURATION = std::chrono::milliseconds(4000);
    zenoh::scout(std::move(config), on_hello, on_good_bye,
                 { .timeout_ms = SCOUT_DURATION.count() });
    done_flag.wait(false);
    std::println("done");

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
