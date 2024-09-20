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
// Example program creates a caching publisher and periodically writes a value on the specified key.
//
// A caching publisher is a publisher with in-memory storage. It exposes a Queryable such that
// querying subscribers can receive past publications. Of course, it also works with regular
// subscribers
//
// Typical usage:
// ```bash
// zenoh_pub_cache [--key=demo/example/test --value="Hello World"]
// ```
//
// Paired with example: zenoh_query_sub.cpp, zenoh_sub.cpp
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-c/blob/master/examples/z_pub_cache.c
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_VALUE = "Put from caching publisher";
    static constexpr auto DEFAULT_KEY = "grape/ipc/example/zenoh/put";

    const auto args_opt =
        grape::conio::ProgramDescription("Cached periodic publisher example")
            .declareOption<std::string>("key", "Key expression", DEFAULT_KEY)
            .declareOption<std::string>("value", "Data to put on the key", DEFAULT_VALUE)
            .parse(argc, argv);

    if (not args_opt.has_value()) {
      throw grape::conio::ProgramOptions::Error{ args_opt.error() };
    }
    const auto& args = args_opt.value();
    const auto key = grape::ipc::ex::getOptionOrThrow<std::string>(args, "key");
    const auto value = grape::ipc::ex::getOptionOrThrow<std::string>(args, "value");

    auto config = zenoh::Config::create_default();

    // enable timestamping
    config.insert_json5(Z_CONFIG_ADD_TIMESTAMP_KEY, "true");

    std::println("Opening session...");
    auto session = zenoh::Session::open(std::move(config));

    //----
    // Note: The rest of this application uses the C API because the C++ API does not expose
    // caching publication yet (Dec 2023)
    //----

    static constexpr auto PUB_HISTORY = 10;
    auto pub_cache_opts = ze_publication_cache_options_default();
    pub_cache_opts.history = PUB_HISTORY;

    std::println("Declaring publication cache on '{}'...", key);
    auto pub_cache =
        ze_declare_publication_cache(session.loan(), z_keyexpr(key.c_str()), &pub_cache_opts);
    if (!z_check(pub_cache)) {
      std::println("Unable to declare publication cache for key expression!");
      return EXIT_FAILURE;
    }

    std::println("Press any key to exit");
    static constexpr auto LOOP_WAIT = std::chrono::seconds(1);
    uint64_t idx = 0;
    while (not grape::conio::kbhit()) {
      const auto msg = std::format("[{}] {}", idx++, value);
      std::println("Publishing Data ('{} : {})", key, msg);
      z_put(session.loan(), z_keyexpr(key.c_str()), reinterpret_cast<const uint8_t*>(msg.data()),
            msg.length(), nullptr);
      std::this_thread::sleep_for(LOOP_WAIT);
    }
    z_drop(z_move(pub_cache));

    return EXIT_SUCCESS;
  } catch (const grape::conio::ProgramOptions::Error& ex) {
    std::ignore = std::fputs(toString(ex).c_str(), stderr);
    return EXIT_FAILURE;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}