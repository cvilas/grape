//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <thread>

#include "grape/conio/conio.h"
#include "grape/conio/program_options.h"
#include "zenoh_utils.h"

//=================================================================================================
// Example program that creates a subscriber. The subscriber will be notified of each put or delete
// made on any key expression matching the subscriber key expression, and will print this
// notification.
//
// Typical usage:
// ```bash
// sub [--key="demo/**"]
// ```
// Paired with example: put.cpp, pub.cpp, pub_shm.cpp, delete.cpp
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/universal/z_sub.cxx
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_KEY = "grape/ipc/example/zenoh/put";

    const auto args_opt =
        grape::conio::ProgramDescription("Subscriber listening for data on specified key")
            .declareOption<std::string>("key", "Key expression", DEFAULT_KEY)
            .parse(argc, argv);

    if (not args_opt.has_value()) {
      grape::panic<grape::Exception>(toString(args_opt.error()));
    }
    const auto& args = args_opt.value();
    const auto key = args.getOptionOrThrow<std::string>("key");

    auto config = zenoh::Config::create_default();

    //-----------------------------------------
    // TODO(vilas): Review and confirm shared memory configuration is correct
    //-----------------------------------------

    // Enable shared-memory on the subscriber to take advantage of publishers on the same host who
    // may be publishing over shared-memory. See also zenoh_pub_shm.cpp
    //
    // Note 1: Shared memory transport is used only if both the publisher and the subscriber are on
    // the same host and are configured to use shared-memory. When on different hosts, they
    // automatically fallback to using the network transport layer.
    //
    // Note 2: With shared-memory enabled, the publisher still uses the network transport layer to
    // notify subscribers of the shared-memory segment to read. Therefore, for very small messages,
    // shared-memory transport could be less efficient than using the default network transport
    // to directly carry the payload
    config.insert_json5("transport/shared_memory/enabled", "true");
    std::println("Opening session...");
    auto session = zenoh::Session::open(std::move(config));

    std::println("Declaring Subscriber on '{}'", key);
    const auto cb = [](const zenoh::Sample& sample) {
      const auto ts = sample.get_timestamp();
      std::println(">> Received {} ('{}' : [{}] '{}')", grape::ipc::ex::toString(sample.get_kind()),
                   sample.get_keyexpr().as_string_view(),
                   (ts ? grape::ipc::ex::toString(ts.value()) : "--no timestamp--"),
                   sample.get_payload().as_string());
    };

    auto subs = session.declare_subscriber(key, cb, zenoh::closures::none);
    std::println("Subscriber on '{}' declared", subs.get_keyexpr().as_string_view());

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
