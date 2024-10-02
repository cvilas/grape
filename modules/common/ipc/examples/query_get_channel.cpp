//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <print>

#include "grape/conio/conio.h"
#include "grape/exception.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program demonstrates how to interact with a queryable to perform a query and block
// wait for an iterator of results _synchronously_ without a callback.
//
// Paired with: queryable.cpp.
// See also: query_get.cpp, query_get_channel_non_blocking.cpp, query_sub.cpp
//
// Derived from:
// https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/zenohc/z_get_channel.cxx
//=================================================================================================

//=================================================================================================
auto main() -> int {
  try {
    static constexpr auto KEY = "grape/ipc/example/zenoh/queryable";
    static constexpr auto PARAM = "default";
    static constexpr auto QUERY = "hello";

    auto config = zenoh::Config::create_default();
    auto session = zenoh::Session::open(std::move(config));

    std::println("Sending Query '{}?{}/{}'...", KEY, PARAM, QUERY);
    static constexpr auto MAX_REPLIES = 16;  //!< handle these many replies
    auto replies = session.get(KEY, PARAM, zenoh::channels::FifoChannel(MAX_REPLIES),
                               { .target = zenoh::QueryTarget::Z_QUERY_TARGET_ALL,
                                 .payload = zenoh::ext::serialize(QUERY) });

    for (auto res = replies.recv(); std::holds_alternative<zenoh::Reply>(res);
         res = replies.recv()) {
      const auto& sample = std::get<zenoh::Reply>(res).get_ok();
      std::println(">> Received ('{}' : '{}')", sample.get_keyexpr().as_string_view(),
                   sample.get_payload().as_string());
    }

    return EXIT_SUCCESS;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}