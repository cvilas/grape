//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <print>
#include <thread>

#include "grape/conio/conio.h"
#include "grape/exception.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program demonstrates how to interact with a queryable to perform a query, and receive
// an iterator of results _asynchronously_ via a callback.
//
// Paired with: queryable.cpp. See also query_get_channel.cpp
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/universal/z_get.cxx
//=================================================================================================

//=================================================================================================
auto main() -> int {
  try {
    static constexpr auto KEY = "grape/ipc/example/zenoh/queryable";
    static constexpr auto PARAM = "default";
    static constexpr auto QUERY = "hello";

    auto config = zenoh::Config::create_default();
    auto session = zenoh::Session::open(std::move(config));

    std::atomic_flag done_flag = ATOMIC_FLAG_INIT;

    // define callback to trigger on response
    const auto on_reply = [](const zenoh::Reply& reply) {
      if (reply.is_ok()) {
        const auto& sample = reply.get_ok();
        const auto& keystr = sample.get_keyexpr();
        std::println(">> Received ('{}': '{}')", keystr.as_string_view(),
                     sample.get_payload().as_string());
      } else {
        std::println("Received error: {}", reply.get_err().get_payload().as_string());
      }
    };

    const auto on_done = [&done_flag]() {
      done_flag.test_and_set();
      done_flag.notify_one();
    };

    // prepare and dispatch request
    session.get(KEY, PARAM, on_reply, on_done,
                { .target = Z_QUERY_TARGET_ALL, .payload = zenoh::ext::serialize(QUERY) });
    done_flag.wait(false);
    std::println("done");

    return EXIT_SUCCESS;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}