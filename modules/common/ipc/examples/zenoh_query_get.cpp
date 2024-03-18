//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <barrier>
#include <print>
#include <thread>

#include "grape/conio/conio.h"
#include "grape/exception.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program demonstrates how to interact with a queryable to perform a query, and receive
// an iterator of results _asynchronously_ via a callback.
//
// Paired with: zenoh_queryable.cpp. See also zenoh_query_sub.cpp, zenoh_query_get_channel.cpp
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-c/blob/master/examples/z_get.c
//=================================================================================================

//=================================================================================================
auto main() -> int {
  try {
    static constexpr auto KEY = "grape/ipc/example/zenoh/queryable";
    static constexpr auto PARAM = "default";
    static constexpr auto QUERY = "hello";

    auto config = zenohc::Config();
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));

    std::barrier sync_point(2);

    // define callback to trigger on response
    const auto reply_cb = [&sync_point](const zenohc::Reply& reply) {
      try {
        const auto sample = grape::ipc::expect<zenohc::Sample>(reply.get());
        const auto& keystr = sample.get_keyexpr();
        std::println(">> Received ('{}': '{}')", keystr.as_string_view(),
                     sample.get_payload().as_string_view());
      } catch (const std::exception& ex) {
        std::println("Exception in reply callback: {}", ex.what());
      }
      // It's possible to get multiple responses to a query. Here we wait only for the first one
      // and signal the main thread to exit as soon as we have it.
      std::ignore = sync_point.arrive();
    };

    // prepare and dispatch request
    auto opts = zenohc::GetOptions();
    opts.set_value(QUERY);
    opts.set_target(Z_QUERY_TARGET_ALL);  //!< query all matching queryables
    session.get(KEY, PARAM, reply_cb, opts);

    sync_point.arrive_and_wait();  //!< wait until reply callback is triggered

    return EXIT_SUCCESS;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}