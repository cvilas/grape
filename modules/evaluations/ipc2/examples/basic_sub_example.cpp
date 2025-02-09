//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <print>
#include <thread>

#include "basic_example_constants.h"
#include "grape/conio/conio.h"
#include "grape/conio/program_options.h"
#include "grape/ipc2/session.h"

//=================================================================================================
// Subscriber example that triggers a callback when data is published on a matching key expression.
//
// Typical usage:
// ```bash
// basic_sub_example [--key="demo/**"]
// ```
// Paired with example: basic_pub_example.cpp
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/universal/z_sub.cxx
//=================================================================================================

namespace {
auto asString(std::span<const std::byte> bytes) -> std::string_view {
  return { // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
           reinterpret_cast<const char*>(bytes.data()),  //
           bytes.size_bytes()
  };
};

}  // namespace

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    const auto maybe_args =
        grape::conio::ProgramDescription("Subscriber listening for data on specified key")
            .declareOption<std::string>("key", "Key expression", grape::ipc2::ex::DEFAULT_TOPIC)
            .parse(argc, argv);

    if (not maybe_args) {
      grape::panic<grape::Exception>(toString(maybe_args.error()));
    }
    const auto& args = maybe_args.value();
    const auto key = args.getOptionOrThrow<std::string>("key");

    std::println("Opening session...");
    auto session = grape::ipc2::Session({});

    std::println("Declaring Subscriber on '{}'", key);
    const auto message_callback = [](std::span<const std::byte> bytes) {
      // Reinterpret message as string
      std::println("{}", asString(bytes));
    };

    auto subs = session.createSubscriber(key, message_callback);

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
