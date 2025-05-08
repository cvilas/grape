//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "grape/app/app.h"
#include "grape/ipc/subscriber.h"
#include "grape/log/syslog.h"

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {  // NOLINT(bugprone-exception-escape)
  // initialise application
  grape::app::init(argc, argv, "GRAPE subscriber application example");

  // dummy data deserializer
  const auto deserialise = [](std::span<const std::byte> bytes) -> std::string {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return { reinterpret_cast<const char*>(bytes.data()), bytes.size() };
  };

  // subsription callback
  const auto callback = [&deserialise](const grape::ipc::Sample& sample) {
    const auto msg = deserialise(sample.data);
    grape::syslog::Info("{}", msg);
  };

  // create subscriber
  static constexpr auto TOPIC = "hello";
  auto sub = grape::ipc::Subscriber(TOPIC, callback);

  std::println(stderr, "Press ctrl-c to exit");
  grape::app::waitForExit();

  return EXIT_SUCCESS;
}
