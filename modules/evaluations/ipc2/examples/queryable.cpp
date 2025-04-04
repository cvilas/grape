//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <thread>

#include "grape/conio/conio.h"
#include "grape/conio/program_options.h"
#include "zenoh_utils.h"

//=================================================================================================
// Example program that declares a queryable. The queryable will be triggered on remote calls on the
// designated topic, and will return a value to the querier.
//
// Paired with examples: query_get.cpp, query_get_channel.cpp
//
// Derived from:
// https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/universal/z_queryable.cxx
//=================================================================================================

//=================================================================================================
auto main() -> int {
  try {
    auto config = zenoh::Config::create_default();
    auto session = zenoh::Session::open(std::move(config));

    static constexpr auto KEY = "grape/ipc2/example/zenoh/queryable";

    std::println("Declaring Queryable on '{}'", KEY);
    const auto on_query = [](const zenoh::Query& query) {
      const auto& keyexpr = query.get_keyexpr();
      const auto params = query.get_parameters();
      const auto payload = query.get_payload();
      std::println(">> Received Query '{}?{}' with value '{}'", keyexpr.as_string_view(), params,
                   payload.has_value() ? payload->get().as_string() : "none");
      const auto* const response = "This is a response from Queryable";
      query.reply(keyexpr, zenoh::ext::serialize(response),
                  { .encoding = zenoh::Encoding("text/plain") });
    };

    const auto on_drop_queryable = []() { std::println("Destroying queryable"); };
    [[maybe_unused]] auto qable = session.declare_queryable(KEY, on_query, on_drop_queryable);

    std::println("Press any key to exit");
    static constexpr auto LOOP_WAIT = std::chrono::milliseconds(100);
    while (not grape::conio::kbhit()) {
      std::this_thread::sleep_for(LOOP_WAIT);
    }

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
