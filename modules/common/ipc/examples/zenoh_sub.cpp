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
// Example program that creates a subscriber. The subscriber will be notified of each put or delete
// made on any key expression matching the subscriber key expression, and will print this
// notification.
//
// Typical usage:
// ```bash
// zenoh_sub [--key="demo/**"]
// ```
// Paired with example: zenoh_put.cpp, zenoh_pub.cpp, zenoh_pub_shm.cpp, zenoh_delete.cpp
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
      throw grape::conio::ProgramOptions::Error{ args_opt.error() };
    }
    const auto& args = args_opt.value();
    const auto key = grape::ipc::ex::getOptionOrThrow<std::string>(args, "key");

    zenohc::Config config;

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
    if (not config.insert_json("transport/shared_memory/enabled", "true")) {
      std::println("Error enabling Shared Memory");
      return EXIT_FAILURE;
    }
    std::println("Opening session...");
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));

    std::println("Declaring Subscriber on '{}'", key);
    const auto cb = [](const zenohc::Sample& sample) {
      std::println(">> Received {} ('{}' : [{}] '{}')", grape::ipc::toString(sample.get_kind()),
                   sample.get_keyexpr().as_string_view(),
                   grape::ipc::toString(sample.get_timestamp()),
                   sample.get_payload().as_string_view());
    };

    auto subs = grape::ipc::expect<zenohc::Subscriber>(session.declare_subscriber(key, cb));
    std::println("Subscriber on '{}' declared", subs.get_keyexpr().as_string_view());

    std::println("Press any key to exit");
    static constexpr auto LOOP_WAIT = std::chrono::milliseconds(100);
    while (not grape::conio::kbhit()) {
      std::this_thread::sleep_for(LOOP_WAIT);
    }
    return EXIT_SUCCESS;
  } catch (const grape::conio::ProgramOptions::Error& ex) {
    /// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::ignore = fprintf(stderr, "Option '%s' %s", ex.key.c_str(), toString(ex.code).data());
    return EXIT_FAILURE;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}
