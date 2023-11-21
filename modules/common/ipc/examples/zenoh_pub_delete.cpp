//
// Copyright (c) 2022 ZettaScale Technology
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <print>

#include "grape/ipc/ipc.h"
#include "grape/utils/command_line_args.h"
#include "grape/utils/conio.h"

//=================================================================================================
// Example program that publishes a sample with Samplekind::Delete. Such messages can be
// interpreted by subscribers and storages however they want, such as "delete all data for keys
// included by this sample's key expression". Run this program with zenoh_sub in another terminal
// to witness the interaction.
//
// Typical usage:
// ```code
// zenoh_pub_delete [--key="demo/**"]
// ```
//
// Paired with example: zenoh_sub.cpp
//
// Derived from:
// https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/universal/z_delete.cxx
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_KEY = "grape/ipc/example/zenoh/put";

    const auto args = grape::utils::CommandLineArgs(argc, argv);
    const auto key_opt = args.getOption<std::string>("key");
    const auto& key = key_opt.has_value() ? key_opt.value() : DEFAULT_KEY;

    zenohc::Config config;
    std::println("Opening session...");
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));

    std::println("Deleting resources matching '{}'...", key);
    zenohc::ErrNo error{};
    if (not session.delete_resource(key, error)) {
      std::println("Delete failed with error {}", error);
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}
