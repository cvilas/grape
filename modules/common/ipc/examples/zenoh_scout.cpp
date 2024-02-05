//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <chrono>
#include <print>
#include <thread>

#include "grape/exception.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// The process of discovering Zenoh applications is called scouting. For a discussion of how
// scouting works in a deployment consisting of peers, clients and routers, see
// https://zenoh.io/docs/getting-started/deployment/
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-c/blob/master/examples/z_scout.c
//=================================================================================================

namespace {

//-------------------------------------------------------------------------------------------------
void onDiscovery(const zenohc::HelloView& hello) {
  std::println("Hello from pid: {}, WhatAmI: {}, locators: {}",
               grape::ipc::toString(hello.get_id()), grape::ipc::toString(hello.get_whatami()),
               grape::ipc::toString(hello.get_locators()));
}

}  // namespace

//=================================================================================================
auto main() -> int {
  try {
    zenohc::ScoutingConfig config;
    std::println("Scouting..");
    const auto success = zenohc::scout(std::move(config), onDiscovery);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::println("{}", success ? "success" : "failed");
    return EXIT_SUCCESS;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}