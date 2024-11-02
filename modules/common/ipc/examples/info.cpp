//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <print>

#include "grape/exception.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program gets unique IDs of endpoints the Zenoh session knows about.
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/universal/z_info.cxx
//=================================================================================================

//=================================================================================================
auto main() -> int {
  try {
    auto config = zenoh::Config::create_default();
    std::println("Opening session...");
    auto session = zenoh::Session::open(std::move(config));

    const auto print_id = [](const zenoh::Id& id) {
      std::println("\t{}", grape::ipc::toString(id));
    };
    std::println("Own id:");
    print_id(session.get_zid());

    std::println("Router ids:");
    for (const auto zid : session.get_routers_z_id()) {
      print_id(zid);
    }

    std::println("Peer ids:");
    for (const auto zid : session.get_peers_z_id()) {
      print_id(zid);
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
