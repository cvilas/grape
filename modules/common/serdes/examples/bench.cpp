//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <stdexcept>

#include <benchmark/benchmark.h>

#include "grape/serdes/serdes.h"
#include "grape/serdes/stream.h"

namespace {

//-------------------------------------------------------------------------------------------------
struct Position {
  double x{};
  double y{};
  double z{};
};

//-------------------------------------------------------------------------------------------------
struct Quaternion {
  double x{};
  double y{};
  double z{};
  double w{};
};

//-------------------------------------------------------------------------------------------------
struct PoseStamped {
  std::int64_t nanoseconds{};
  Position position{};
  Quaternion orientation{};
};

constexpr auto BUFFER_INIT_SIZE = 1024U;
using OutStream = grape::serdes::OutStream<BUFFER_INIT_SIZE>;
using InStream = grape::serdes::InStream;
using Serialiser = grape::serdes::Serialiser<OutStream>;
using Deserialiser = grape::serdes::Deserialiser<InStream>;

//-------------------------------------------------------------------------------------------------
void bmSerialize(benchmark::State& state) {
  const auto pos = PoseStamped();
  auto buf = OutStream();

  for (auto st : state) {
    (void)st;
    buf.reset();
    auto serializer = Serialiser(buf);
    if (not serializer.pack(pos)) {
      throw std::runtime_error("Serialisation error");
    }

    benchmark::DoNotOptimize(buf.data());
    benchmark::ClobberMemory();
  }
}

//-------------------------------------------------------------------------------------------------
void bmDeserialize(benchmark::State& state) {
  const auto pos = PoseStamped();
  auto obuf = OutStream();
  auto serializer = Serialiser(obuf);
  if (not serializer.pack(pos)) {
    throw std::runtime_error("Serialisation error");
  }

  for (auto st : state) {
    (void)st;
    auto ibuf = InStream(obuf.data());
    auto deserializer = Deserialiser(ibuf);
    PoseStamped deserialized_pose;
    if (not deserializer.unpack(deserialized_pose)) {
      throw std::runtime_error("Deserialisation error");
    }

    benchmark::DoNotOptimize(deserialized_pose);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(bmSerialize);
BENCHMARK(bmDeserialize);

}  // namespace

BENCHMARK_MAIN();
