//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <print>
#include <thread>

#include "grape/exception.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program demonstrates how to interact with a queryable to perform a query and
// non-blocking wait for an iterator of results _synchronously_ without a callback.
//
// Paired with: queryable.cpp.
// See also: query_get.cpp, query_get_channel.cpp, query_sub.cpp
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

    auto poll = true;
    while (poll) {
      const auto res = replies.try_recv();

      // handle response
      if (std::holds_alternative<zenoh::Reply>(res)) {
        const auto& sample = std::get<zenoh::Reply>(res).get_ok();
        std::println(">> Received ('{}' : '{}')", sample.get_keyexpr().as_string_view(),
                     sample.get_payload().as_string());
      }

      // handle error
      if (std::holds_alternative<zenoh::channels::RecvError>(res)) {
        switch (std::get<zenoh::channels::RecvError>(res)) {
          case zenoh::channels::RecvError::Z_NODATA: {
            std::println("No data available yet");
            static constexpr auto WAIT_FOR_REPLY = std::chrono::milliseconds(1000);
            std::this_thread::sleep_for(WAIT_FOR_REPLY);
            break;
          }
          case zenoh::channels::RecvError::Z_DISCONNECTED: {
            std::println("Channel disconnected");
            poll = false;
            break;
          }
        }  // error type
      }  // recv error
    }  // while

    return EXIT_SUCCESS;

  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
