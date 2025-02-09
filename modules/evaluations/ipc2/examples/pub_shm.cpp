//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <cstring>
#include <thread>

#include "grape/conio/program_options.h"
#include "zenoh_utils.h"

//=================================================================================================
// Example program creates a publisher on shared memory and periodically writes a value on the
// specified key. The published value will be received by all matching subscribers.
//
// Typical usage:
// ```bash
// pub_shm [--key=demo/example/test --value="Hello World"]
// ```
//
// Paired with example: sub.cpp
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/zenohc/z_pub_shm.cxx
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_VALUE = "Put from Zenoh C++!";
    static constexpr auto DEFAULT_KEY = "grape/ipc2/example/zenoh/put";

    const auto args_opt =
        grape::conio::ProgramDescription("Periodic shared-memory publisher example")
            .declareOption<std::string>("key", "Key expression", DEFAULT_KEY)
            .declareOption<std::string>("value", "Data to put on the key", DEFAULT_VALUE)
            .parse(argc, argv);

    if (not args_opt.has_value()) {
      grape::panic<grape::Exception>(toString(args_opt.error()));
    }
    const auto& args = args_opt.value();
    const auto key = args.getOptionOrThrow<std::string>("key");
    const auto value = args.getOptionOrThrow<std::string>("value");

    auto config = zenoh::Config::create_default();

    //-----------------------------------------
    // TODO(vilas): Review and confirm shared memory configuration is correct
    //-----------------------------------------

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
    config.insert_json5("transport/shared_memory/enabled", "true");

    std::println("Opening session...");
    auto session = zenoh::Session::open(std::move(config));

    std::println("Declaring Publisher on '{}'...", key);
    auto pub = session.declare_publisher(key);

    std::println("Creating shared-memory manager...");
    static constexpr auto SHM_SIZE = 65536U;
    static constexpr auto SHM_ALIGN = 2U;
    auto shm_provider = zenoh::PosixShmProvider(
        zenoh::MemoryLayout(SHM_SIZE, zenoh::AllocAlignment({ SHM_ALIGN })));

    std::println("Press CTRL-C to quit...");
    static constexpr auto LOOP_WAIT = std::chrono::seconds(1);
    for (int idx = 0; idx < std::numeric_limits<int>::max(); ++idx) {
      std::this_thread::sleep_for(LOOP_WAIT);
      const auto msg = std::format("[{}] {}", idx, value);
      std::println("Putting Data ('{}' : '{}')", key, msg);

      std::println("Allocating SHM buffer...");
      const auto len = msg.length();
      auto alloc_result = shm_provider.alloc_gc_defrag_blocking(len, zenoh::AllocAlignment({ 0 }));
      auto&& buf = std::get<zenoh::ZShmMut>(std::move(alloc_result));
      std::memcpy(buf.data(), msg.data(), len);
      pub.put(std::move(buf), { .encoding = zenoh::Encoding("text/plain") });
    }

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
