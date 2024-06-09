//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <algorithm>
#include <print>
#include <thread>

#include "examples_utils.h"
#include "grape/exception.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program creates a publisher on shared memory and periodically writes a value on the
// specified key. The published value will be received by all matching subscribers.
//
// Typical usage:
// ```bash
// zenoh_pub_shm [--key=demo/example/test --value="Hello World"]
// ```
//
// Paired with example: zenoh_sub.cpp
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/zenohc/z_pub_shm.cxx
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr size_t NUM_PUTS = 10;
    static constexpr auto DEFAULT_VALUE = "Put from Zenoh C++!";
    static constexpr auto DEFAULT_KEY = "grape/ipc/example/zenoh/put";

    const auto args_opt =
        grape::conio::ProgramDescription("Periodic shared-memory publisher example")
            .declareOption<std::string>("key", "Key expression", DEFAULT_KEY)
            .declareOption<std::string>("value", "Data to put on the key", DEFAULT_VALUE)
            .parse(argc, argv);

    if (not args_opt.has_value()) {
      throw grape::conio::ProgramOptions::Error{ args_opt.error() };
    }
    const auto& args = args_opt.value();
    const auto key = grape::ipc::ex::getOptionOrThrow<std::string>(args, "key");
    const auto value = grape::ipc::ex::getOptionOrThrow<std::string>(args, "value");

    zenohc::Config config;

    // Enable the publisher to operate over shared-memory. Note that shared-memory should also be
    // enabled on the subscriber side (see zenoh_sub.cpp).
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

    std::println("Creating shared-memory manager...");
    const auto shm_id = grape::ipc::toString(session.info_zid());
    static constexpr size_t MAX_MSG_SIZE = 256;
    static constexpr auto SHM_SIZE = NUM_PUTS * MAX_MSG_SIZE;
    auto shm_manager =
        grape::ipc::expect<zenohc::ShmManager>(shm_manager_new(session, shm_id.c_str(), SHM_SIZE));

    std::println("Declaring Publisher on '{}'...", key);
    auto pub = grape::ipc::expect<zenohc::Publisher>(session.declare_publisher(key));

    // Get shared-memory segments, write data into it, then publish
    zenohc::PublisherPutOptions options;
    options.set_encoding(Z_ENCODING_PREFIX_TEXT_PLAIN);
    static constexpr auto LOOP_WAIT = std::chrono::seconds(1);
    for (size_t idx = 0; idx < NUM_PUTS; ++idx) {
      auto shm_buf = grape::ipc::expect<zenohc::Shmbuf>(shm_manager.alloc(MAX_MSG_SIZE));
      const auto msg = std::format("[{}] {}", idx, value);
      std::copy_n(msg.begin(), std::min(MAX_MSG_SIZE, msg.length()), shm_buf.ptr());
      std::println("Putting Data ('{}' : '{}')", key, shm_buf.as_string_view());
      auto payload = shm_buf.into_payload();
      pub.put_owned(std::move(payload), options);
      std::this_thread::sleep_for(LOOP_WAIT);
    }
    return EXIT_SUCCESS;
  } catch (const grape::conio::ProgramOptions::Error& ex) {
    grape::ipc::ex::printMessage(ex);
    return EXIT_FAILURE;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}