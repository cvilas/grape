//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <print>
#include <thread>

#include "grape/conio/conio.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program to demonstrate a late joining subscriber interacting with a queryable.
//
// A QueryingSubscriber is useful for late-joining subscribers: given that a publication cache has
// been instantiated on a keyexpr, the querying subscriber will first perform a get to obtain the
// "existing" state, and then keep on behaving like a normal subscriber (letting you force another
// get through its fetch method).
//
// Paired with example: zenoh_queryable.cpp.
// See also: zenoh_query_get.cpp, zenoh_query_get_channel_non_blocking.cpp,
// zenoh_query_get_channel.cpp
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-c/blob/master/examples/z_query_sub.c
//=================================================================================================

//-------------------------------------------------------------------------------------------------
void dataHandler(const z_sample_t* sample, void* arg) {
  (void)arg;
  z_owned_str_t keystr = z_keyexpr_to_string(sample->keyexpr);
  const auto str =
      std::string(reinterpret_cast<const char*>(sample->payload.start), sample->payload.len);
  std::println(">> Received {} ('{}': '{}')", grape::ipc::toString(sample->kind), z_loan(keystr),
               str);
  z_drop(z_move(keystr));
}

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  (void)argc;
  (void)argv;
  try {
    auto config = zenohc::Config();
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));

    static constexpr auto KEY = "grape/ipc/example/zenoh/queryable";
    std::println("Declaring querying subscriber on '{}'...", KEY);

    //----
    // Note: The rest of this application uses the C API because the C++ API does not expose
    // querying subscriber yet (Dec 2023)
    //
    // TODO: This example does not work as in the description above. Make it so. Clarify how to
    // force another 'get'
    //----

    const auto sub_opts = ze_querying_subscriber_options_default();
    z_owned_closure_sample_t callback = z_closure(dataHandler);
    auto sub =
        ze_declare_querying_subscriber(session.loan(), z_keyexpr(KEY), z_move(callback), &sub_opts);
    if (!z_check(sub)) {
      std::println("Unable to declare querying subscriber.");
      return EXIT_FAILURE;
    }

    std::println("Press any key to exit");
    static constexpr auto LOOP_WAIT = std::chrono::milliseconds(100);
    while (not grape::conio::kbhit()) {
      std::this_thread::sleep_for(LOOP_WAIT);
    }

    z_drop(z_move(sub));

    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}
