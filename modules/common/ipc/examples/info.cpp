//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <print>

#include "grape/conio/program_options.h"
#include "grape/exception.h"
#include "grape/ipc/session.h"

//=================================================================================================
// Example program gets unique IDs of endpoints the Zenoh session knows about.
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/universal/z_info.cxx
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    const auto maybe_args = grape::conio::ProgramDescription(
                                "Prints unique IDs of all endpoints this session is aware of")
                                .parse(argc, argv);
    if (not maybe_args) {
      grape::panic<grape::Exception>(toString(maybe_args.error()));
    }
    std::println("Opening session...");
    auto session = grape::ipc::Session({});

    const auto print_id = [](const grape::ipc::UUID& id) {
      std::println("\t{}", grape::ipc::toString(id));
    };
    std::println("Own id:");
    print_id(session.id());

    std::println("Router ids:");
    for (const auto id : session.routers()) {
      print_id(id);
    }

    std::println("Peer ids:");
    for (const auto id : session.peers()) {
      print_id(id);
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
