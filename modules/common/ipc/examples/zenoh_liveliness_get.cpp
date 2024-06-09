//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <print>
#include <thread>

#include "examples_utils.h"
#include "grape/exception.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Queries currently alive tokens that match a given key expression. To learn how tokens are
// declared, see zenoh_liveliness_declare.
//
// Typical usage:
// ```bash
// zenoh_liveliness_get [--key=my/key/expression/**]
// ```
//
// Paired with example: zenoh_liveliness_declare, zenoh_liveliness_sub
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-c/blob/master/examples/z_get_liveliness.c
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_KEY = "grape/ipc/example/zenoh/**";

    const auto args_opt =
        grape::conio::ProgramDescription("Queries liveliness token")
            .declareOption<std::string>("key", "key expression to query liveliness of", DEFAULT_KEY)
            .parse(argc, argv);

    if (not args_opt.has_value()) {
      throw grape::conio::ProgramOptions::Error{ args_opt.error() };
    }
    const auto& args = args_opt.value();

    zenohc::Config config;
    std::println("Opening session...");
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));

    //----
    // Note: The rest of this application uses the C API because the C++ API does not expose
    // liveliness yet (Dec 2023)
    //----

    const auto key = grape::ipc::ex::getOptionOrThrow<std::string>(args, "key");
    const auto keystr = z_keyexpr(key.c_str());
    std::println("Sending liveliness query for '{}'...", key);
    static constexpr auto FIFO_BOUND = 16;
    z_owned_reply_channel_t channel = zc_reply_fifo_new(FIFO_BOUND);
    zc_liveliness_get(session.loan(), keystr, z_move(channel.send), nullptr);
    z_owned_reply_t reply = z_reply_null();
    for (z_call(channel.recv, &reply); z_check(reply); z_call(channel.recv, &reply)) {
      if (z_reply_is_ok(&reply)) {
        z_sample_t sample = z_reply_ok(&reply);
        z_owned_str_t sample_keystr = z_keyexpr_to_string(sample.keyexpr);
        std::println(">> Alive token ('{}')", z_loan(sample_keystr));
        z_drop(z_move(sample_keystr));
      } else {
        std::println("Received an error");
      }
    }
    z_drop(z_move(reply));
    z_drop(z_move(channel));
    return EXIT_SUCCESS;
  } catch (const grape::conio::ProgramOptions::Error& ex) {
    grape::ipc::ex::printMessage(ex);
    return EXIT_FAILURE;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}