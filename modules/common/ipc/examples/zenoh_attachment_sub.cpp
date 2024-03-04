
//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <print>
#include <thread>

#include "examples_utils.h"
#include "grape/conio/conio.h"
#include "grape/exception.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program that subscribes to data as well as their attachments
//
// Typical usage:
// ```bash
// zenoh_attachment_sub [--key="demo/**"]
// ```
//
// Paired with example: zenoh_attachment_pub.cpp
//
// Derived from:
// https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/universal/z_sub_attachment.cxx
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

    std::println("Opening session...");
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));

    const auto cb = [](const zenohc::Sample& sample) {
      std::println(">> Received {} ('{}' : [{}] '{}')", grape::ipc::toString(sample.get_kind()),
                   sample.get_keyexpr().as_string_view(),
                   grape::ipc::toString(sample.get_timestamp()),
                   sample.get_payload().as_string_view());
      const auto& attachments = sample.get_attachment();
      if (attachments.check()) {
        attachments.iterate([](const zenohc::BytesView& k, const zenohc::BytesView& v) -> bool {
          std::println("   attachment: {}: {}", k.as_string_view(), v.as_string_view());
          return true;
        });

        // reads particular attachment item
        auto index = attachments.get("index");
        if (index != "") {
          std::println("   message number: {}", index.as_string_view());
        }
      }
    };

    std::println("Declaring Subscriber on '{}'", key);
    auto subs = grape::ipc::expect<zenohc::Subscriber>(session.declare_subscriber(key, cb));
    std::println("Subscriber on '{}' declared", subs.get_keyexpr().as_string_view());

    std::println("Press any key to exit");
    static constexpr auto LOOP_WAIT = std::chrono::milliseconds(100);
    while (not grape::conio::kbhit()) {
      std::this_thread::sleep_for(LOOP_WAIT);
    }
    return EXIT_SUCCESS;
  } catch (const grape::conio::ProgramOptions::Error& ex) {
    std::ignore = std::fputs(toString(ex).c_str(), stderr);
    return EXIT_FAILURE;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}
