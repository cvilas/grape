
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
// attachment_sub [--key="demo/**"]
// ```
//
// Paired with example: attachment_pub.cpp
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

    auto config = zenoh::Config::create_default();

    std::println("Opening session...");
    auto session = zenoh::Session::open(std::move(config));

    const auto cb = [](const zenoh::Sample& sample) {
      const auto ts = sample.get_timestamp();

      std::println(">> Received {} ('{}' : [{}] '{}')", grape::ipc::toString(sample.get_kind()),
                   sample.get_keyexpr().as_string_view(),
                   (ts ? grape::ipc::toString(ts.value()) : "--no timestamp--"),
                   sample.get_payload().as_string());
      const auto maybe_attachments = sample.get_attachment();
      if (maybe_attachments.has_value()) {
        const auto attachments =
            zenoh::ext::deserialize<std::unordered_map<std::string, std::string>>(
                maybe_attachments->get());
        for (auto&& [k, v] : attachments) {
          std::println("   attachment: {}: {}", k, v);
        }
      }
      return true;
    };

    std::println("Declaring Subscriber on '{}'", key);
    auto subs = session.declare_subscriber(key, cb, zenoh::closures::none);
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