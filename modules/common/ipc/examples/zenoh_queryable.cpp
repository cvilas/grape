//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <print>
#include <thread>

#include "grape/conio/conio.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program that declares a queryable. The queryable will be triggered on remote calls on the
// designated topic, and will return a value to the querier.
//
// Paired with examples: zenoh_query_get.cpp, zenoh_query_get_channel.cpp, zenoh_query_sub.cpp
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-c/blob/master/examples/z_queryable.c
//=================================================================================================

namespace {

//-------------------------------------------------------------------------------------------------
void queryHandler(const zenohc::Query& query) {
  const auto keystr = query.get_keyexpr();
  const auto pred = query.get_parameters();
  const auto payload = query.get_value();
  std::println(">> Received Query '{}?{}' with value '{}'", keystr.as_string_view(),
               pred.as_string_view(), payload.get_payload().as_string_view());
  auto options = zenohc::QueryReplyOptions();
  options.set_encoding(Z_ENCODING_PREFIX_TEXT_PLAIN);
  static constexpr auto QUERY_RESULTS = "This is a response from Queryable";
  query.reply(keystr, QUERY_RESULTS, options);
}

}  // namespace

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  (void)argc;
  (void)argv;
  try {
    auto config = zenohc::Config();
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));

    static constexpr auto KEY = "grape/ipc/example/zenoh/queryable";

    std::println("Declaring Queryable on '{}'", KEY);
    auto qable =
        grape::ipc::expect<zenohc::Queryable>(session.declare_queryable(KEY, queryHandler));

    std::println("Press any key to exit");
    static constexpr auto LOOP_WAIT = std::chrono::milliseconds(100);
    while (not grape::conio::kbhit()) {
      std::this_thread::sleep_for(LOOP_WAIT);
    }

    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}
