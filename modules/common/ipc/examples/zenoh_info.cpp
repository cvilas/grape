//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <print>
#include <span>

#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program gets unique IDs of endpoints the Zenoh session knows about.
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-c/blob/master/examples/z_info.c
//=================================================================================================

//=================================================================================================
auto main(int argc, char** argv) -> int {
  (void)argc;
  (void)argv;
  try {
    zenohc::Config config;
    std::println("Opening session...");
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));

    const auto print_id = [](const zenohc::Id& id) {
      std::println("\t{}", grape::ipc::toString(id));
    };
    std::println("Own id:");
    print_id(session.info_zid());

    std::println("Router ids:");
    session.info_routers_zid(print_id);

    std::println("Peer ids:");
    session.info_peers_zid(print_id);
    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}
