//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <thread>

#include "grape/conio/conio.h"
#include "grape/conio/program_options.h"
#include "zenoh_utils.h"

//=================================================================================================
// Example program creates a caching publisher and periodically writes a value on the specified key.
//
// A caching publisher is a publisher with in-memory storage. It exposes a Queryable such that
// querying subscribers can receive past publications. Of course, it also works with regular
// subscribers
//
// Typical usage:
// ```bash
// pub_cache [--key=demo/example/test --value="Hello World"]
// ```
//
// Paired with example: query_sub.cpp, sub.cpp
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-c/blob/master/examples/z_pub_cache.c
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_VALUE = "Put from caching publisher";
    static constexpr auto DEFAULT_KEY = "grape/ipc2/example/zenoh/put";

    const auto args_opt =
        grape::conio::ProgramDescription("Cached periodic publisher example")
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

    // enable timestamping
    config.insert_json5(Z_CONFIG_ADD_TIMESTAMP_KEY, "true");

    std::println("Opening session...");
    auto session = zenoh::Session::open(std::move(config));

    static constexpr auto PUB_HISTORY = 10;
    auto pub_cache_opts = zenoh::Session::PublicationCacheOptions();
    pub_cache_opts.history = PUB_HISTORY;

    std::println("Declaring publication cache on '{}'...", key);
    auto pub_cache = session.declare_publication_cache(key, std::move(pub_cache_opts));

    std::println("Press any key to exit");
    static constexpr auto LOOP_WAIT = std::chrono::seconds(1);
    uint64_t idx = 0;
    while (not grape::conio::kbhit()) {
      const auto msg = std::format("[{}] {}", idx++, value);
      std::println("Publishing Data ('{} : {})", key, msg);
      session.put(key, zenoh::ext::serialize(msg), { .encoding = zenoh::Encoding("text/plain") });
      std::this_thread::sleep_for(LOOP_WAIT);
    }

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
