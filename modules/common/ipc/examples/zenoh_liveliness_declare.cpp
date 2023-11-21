//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <print>
#include <thread>

#include "grape/ipc/ipc.h"
#include "grape/utils/command_line_args.h"
#include "grape/utils/conio.h"

//=================================================================================================
// Declares a liveliness token on a given key expression. This token will be seen alive by the
// zenoh_get_liveliness and zenoh_sub_liveliness until user explicitely drops the token by
// pressing 'd' or implicitely dropped by terminating or killing this example.
//
// Typical usage:
// ```bash
// zenoh_liveliness_declare [--key=group1/member1]
// ```
//
// Paired with example: zenoh_liveliness_get, zenoh_liveliness_sub
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-c/blob/master/examples/z_liveliness.c
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_KEY = "grape/ipc/example/zenoh/liveliness";

    const auto args = grape::utils::CommandLineArgs(argc, argv);
    const auto key_opt = args.getOption<std::string>("key");
    const auto& key = key_opt.has_value() ? key_opt.value() : DEFAULT_KEY;

    zenohc::Config config;
    std::println("Opening session...");
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));

    //----
    // Note: The rest of this application uses the C API because the C++ API does not expose
    // liveliness yet (Dec 2023)
    //----

    auto token = zc_owned_liveliness_token_t();
    const auto toggle_liveliness = [&session, &token, &key] {
      if (!z_check(token)) {
        token = zc_liveliness_declare_token(session.loan(), z_keyexpr(key.c_str()), nullptr);
        std::println("Liveliness token {}", z_check(token) ? "declared" : "declaration failed");
      } else {
        z_drop(z_move(token));
        std::println("Liveliness token undeclared");
      }
    };

    toggle_liveliness();

    std::println("Enter 'd' to undeclare/declare liveliness token for '{}', 'q' to quit...", key);
    while (true) {
      const auto c = grape::utils::kbhit() ? grape::utils::getch() : 0;
      if (c == 'q') {
        break;
      }
      if (c == 'd') {
        toggle_liveliness();
      }
      static constexpr auto LOOP_WAIT = std::chrono::milliseconds(100);
      std::this_thread::sleep_for(LOOP_WAIT);
    }
    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}
