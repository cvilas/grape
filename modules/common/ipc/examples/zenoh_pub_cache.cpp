//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <print>
#include <thread>

#include "grape/conio/conio.h"
#include "grape/conio/program_options.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program creates a publisher and periodically writes a value on the specified key. The
// published value will be received by all matching subscribers.
//
// Typical usage:
// ```bash
// zenoh_pub_cache [--key=demo/example/test --value="Hello World"]
// ```
//
// Paired with example: zenoh_sub.cpp
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-c/blob/master/examples/z_pub_cache.c
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_VALUE = "Put from caching publisher";
    static constexpr auto DEFAULT_KEY = "grape/ipc/example/zenoh/put";

    auto desc = grape::conio::ProgramDescription("Cached periodic publisher example");
    desc.declareOption<std::string>("key", "Key expression", DEFAULT_KEY)
        .declareOption<std::string>("value", "Data to put on the key", DEFAULT_VALUE);

    const auto args = std::move(desc).parse(argc, argv);
    const auto key = args.getOption<std::string>("key");
    const auto value = args.getOption<std::string>("value");

    zenohc::Config config;

    // enable timestamping
    if (not config.insert_json(Z_CONFIG_ADD_TIMESTAMP_KEY, "true")) {
      std::println("Error enabling timestamps");
      return EXIT_FAILURE;
    }

    std::println("Opening session...");
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));

    static constexpr auto PUB_HISTORY = 42;
    auto pub_cache_opts = ze_publication_cache_options_default();
    pub_cache_opts.history = PUB_HISTORY;

    std::println("Declaring publication cache on '{}'...", key);
    auto pub_cache =
        ze_declare_publication_cache(session.loan(), z_keyexpr(key.c_str()), &pub_cache_opts);
    if (!z_check(pub_cache)) {
      std::println("Unable to declare publication cache for key expression!");
      return EXIT_FAILURE;
    }

    //-----
    // TODO: make this example work
    //-----

    std::println("Press any key to exit");
    static constexpr auto LOOP_WAIT = std::chrono::seconds(1);
    uint64_t idx = 0;
    while (not grape::conio::kbhit()) {
      const auto msg = std::format("[{}] {}", idx++, value);
      std::println("Publishing Data ('{} : {})", key, msg);
      z_put(session.loan(), z_keyexpr(key.c_str()), reinterpret_cast<const uint8_t*>(msg.data()),
            msg.length(), nullptr);
      // session.put(key, msg);
      std::this_thread::sleep_for(LOOP_WAIT);
    }
    z_drop(z_move(pub_cache));

    return EXIT_SUCCESS;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}