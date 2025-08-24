//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "advanced_example.h"

#include <print>

#include "grape/serdes/stream.h"
#include "grape/time.h"

//=================================================================================================
/// Demonstrates serialisation of a complex data structure
auto main(int argc, const char* argv[]) -> int {
  (void)argc;
  (void)argv;

  constexpr auto BUF_SIZE = 1024U;
  using OutStream = grape::serdes::OutStream<BUF_SIZE>;
  using InStream = grape::serdes::InStream;
  using Serialiser = grape::serdes::Serialiser<OutStream>;
  using Deserialiser = grape::serdes::Deserialiser<InStream>;

  try {
    const auto pose =
        PoseStamped{ .nanoseconds = grape::SystemClock::toNanos(grape::SystemClock::now()),
                     .position = { .x = 0.01, .y = 2.0, .z = 10.0 },
                     .orientation = { .x = 0.01, .y = 0.03, .z = 0.1, .w = 1 } };

    // serialise
    auto ostream = OutStream();
    auto serialiser = Serialiser(ostream);
    if (not serialiser.pack(pose)) {
      std::println("Serialisation error. Exiting");
      return EXIT_FAILURE;
    }
    std::println("Serialised into {} bytes", ostream.size());

    // deserialise
    auto istream = InStream(ostream.data());
    auto pose2 = PoseStamped{};
    auto deserialiser = Deserialiser(istream);
    if (not deserialiser.unpack(pose2)) {
      std::println("Deserialisation error. Exiting");
      return EXIT_FAILURE;
    }

    // compare
    std::println("Original pose: timestamp={}, position=[{}, {}, {}], orientation=[{}, {}, {}, {}]",
                 grape::SystemClock::fromNanos(pose.nanoseconds), pose.position.x, pose.position.y,
                 pose.position.z, pose.orientation.x, pose.orientation.y, pose.orientation.z,
                 pose.orientation.w);

    std::println(
        "Recovered pose: timestamp={}, position=[{}, {}, {}], orientation=[{}, {}, {}, {}]",
        grape::SystemClock::fromNanos(pose.nanoseconds), pose2.position.x, pose2.position.y,
        pose2.position.z, pose2.orientation.x, pose2.orientation.y, pose2.orientation.z,
        pose2.orientation.w);

    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}
