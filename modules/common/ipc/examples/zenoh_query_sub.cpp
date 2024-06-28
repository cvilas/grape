//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <print>
#include <thread>

#include "examples_utils.h"
#include "grape/conio/conio.h"
#include "grape/exception.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program to demonstrate querying subscriber.
//
// A querying subscriber queries, then subscribes. When matched with a caching publisher, the
// subscriber first performs a get to obtain the "existing" state, and thereafter continues
// behaving like a regular subscriber. Caching publisher plus querying subscribers are useful where
// a subscriber might join late (after publisher has started), but must receive historical data
// from a publisher in order to initialise and function.
//
// Paired with example: zenoh_pub_cache.cpp
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-c/blob/master/examples/z_query_sub.c
//=================================================================================================

namespace {

//-------------------------------------------------------------------------------------------------
void dataHandler(const z_sample_t* sample, void* arg) {
  (void)arg;
  z_owned_str_t keystr = z_keyexpr_to_string(sample->keyexpr);
  const auto payload =
      std::string(reinterpret_cast<const char*>(sample->payload.start), sample->payload.len);
  std::println(">> Received {} ('{}' : [{}] '{}')", grape::ipc::toString(sample->kind),
               z_loan(keystr), grape::ipc::toString(sample->timestamp), payload);

  z_drop(z_move(keystr));
}

}  // namespace

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_KEY = "grape/ipc/example/zenoh/put";

    const auto args_opt = grape::conio::ProgramDescription("Querying subscriber example")
                              .declareOption<std::string>("key", "Key expression", DEFAULT_KEY)
                              .parse(argc, argv);

    if (not args_opt.has_value()) {
      throw grape::conio::ProgramOptions::Error{ args_opt.error() };
    }
    const auto& args = args_opt.value();
    const auto key = grape::ipc::ex::getOptionOrThrow<std::string>(args, "key");
    std::println("Declaring querying subscriber on '{}'...", key);

    auto config = zenohc::Config();
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));

    //----
    // Note: The rest of this application uses the C API because the C++ API does not expose
    // querying subscriber yet (Dec 2023)
    //----

    const auto sub_opts = ze_querying_subscriber_options_default();
    z_owned_closure_sample_t callback = z_closure(dataHandler);
    auto sub = ze_declare_querying_subscriber(session.loan(), z_keyexpr(key.c_str()),
                                              z_move(callback), &sub_opts);
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
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}
