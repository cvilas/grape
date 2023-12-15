//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <print>
#include <thread>

#include "grape/conio/conio.h"
#include "grape/conio/program_options.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Demonstrative example that subscribes to liveliness changes on tokens that match a given key
// expression (grape/ipc/example/zenoh/** by default). To learn how tokens are declared, see
// zenoh_liveliness_declare.
//
// Note: This example will not receive information about matching liveliness tokens that were alive
// before it's start.
//
// Typical usage:
// ```bash
// zenoh_liveliness_sub [--key=key/expression/**]
// ```
//
// Paired with example: zenoh_liveliness_declare, zenoh_liveliness_sub
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-c/blob/master/examples/z_sub_liveliness.c
//=================================================================================================

//-------------------------------------------------------------------------------------------------
void dataHandler(const z_sample_t* sample, void* arg) {
  (void)arg;
  z_owned_str_t keystr = z_keyexpr_to_string(sample->keyexpr);
  switch (sample->kind) {
    case Z_SAMPLE_KIND_PUT:
      std::println(">> New alive token ('{}')", z_loan(keystr));
      break;
    case Z_SAMPLE_KIND_DELETE:
      std::println(">> Dropped token ('{}')", z_loan(keystr));
      break;
  }
  z_drop(z_move(keystr));
}

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_KEY = "grape/ipc/example/zenoh/**";

    auto desc = grape::conio::ProgramDescription("Subscribes to liveliness state changes");
    desc.defineOption<std::string>("key", "key expression to track liveliness of", DEFAULT_KEY);

    const auto args = std::move(desc).parse(argc, argv);
    const auto key = args.getOption<std::string>("key");

    zenohc::Config config;
    std::println("Opening session...");
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));

    //----
    // Note: The rest of this application uses the C API because the C++ API does not expose
    // liveliness yet (Dec 2023)
    //----

    std::println("Declaring liveliness subscriber on '{}'...", key);
    const auto keystr = z_keyexpr(key.c_str());
    z_owned_closure_sample_t callback = z_closure(dataHandler);
    auto sub = zc_liveliness_declare_subscriber(session.loan(), keystr, z_move(callback), nullptr);
    if (!z_check(sub)) {
      std::println("Unable to declare liveliness subscriber.");
      return EXIT_FAILURE;
    }

    std::println("Enter 'q' to quit...");
    while (true) {
      const auto c = grape::conio::kbhit() ? grape::conio::getch() : 0;
      if (c == 'q') {
        break;
      }
      static constexpr auto LOOP_WAIT = std::chrono::milliseconds(100);
      std::this_thread::sleep_for(LOOP_WAIT);
    }

    z_undeclare_subscriber(z_move(sub));
    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}
