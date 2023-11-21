//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <print>

#include "grape/ipc/ipc.h"
#include "grape/utils/conio.h"

//=================================================================================================
// Example program demonstrates how to interact with a queryable to perform a query and block
// wait for an iterator of results _synchronously_ without a callback.
//
// Paired with: zenoh_queryable.cpp.
// See also: zenoh_query_get.cpp, zenoh_query_get_channel_non_blocking.cpp, zenoh_query_sub.cpp
//
// Derived from:
// https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/zenohc/z_get_channel.cxx
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  (void)argc;
  (void)argv;
  try {
    static constexpr auto KEY = "grape/ipc/example/zenoh/queryable";
    static constexpr auto PARAM = "default";
    static constexpr auto QUERY = "hello";

    auto config = zenohc::Config();
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));

    std::println("Sending Query '{}?{}/{}'...", KEY, PARAM, QUERY);
    auto opts = zenohc::GetOptions();
    opts.set_value(QUERY);
    opts.set_target(Z_QUERY_TARGET_ALL);  //!< query all matching queryables

    static constexpr auto MAX_REPLIES = 16;  //!< handle these many replies
    auto [send, recv] = zenohc::reply_fifo_new(MAX_REPLIES);
    if (not session.get(KEY, PARAM, std::move(send), opts)) {
      std::println("Query unsuccessful");
      return EXIT_FAILURE;
    }

    auto reply = zenohc::Reply(nullptr);
    for (recv(reply); reply.check(); recv(reply)) {
      try {
        auto sample = grape::ipc::expect<zenohc::Sample>(reply.get());
        std::println(">> Received ('{}' : '{}')", sample.get_keyexpr().as_string_view(),
                     sample.get_payload().as_string_view());
      } catch (const std::exception& ex) {
        std::println("Exception in reply: {}", ex.what());
      }
    }
    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}