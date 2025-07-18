//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <stdexcept>

#include <benchmark/benchmark.h>

#include "advanced_example.h"
#include "grape/serdes/stream.h"

namespace {

constexpr auto BUFFER_INIT_SIZE = 1024U;

//-------------------------------------------------------------------------------------------------
void bmSerialize(benchmark::State& state) {
  const auto pos = PoseStamped();
  auto buf = grape::serdes::OutStream<BUFFER_INIT_SIZE>();

  for (auto st : state) {
    (void)st;
    auto serializer = grape::serdes::Serialiser(buf);
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
  auto obuf = grape::serdes::OutStream<BUFFER_INIT_SIZE>();
  auto serializer = grape::serdes::Serialiser(obuf);
  if (not serializer.pack(pos)) {
    throw std::runtime_error("Serialisation error");
  }

  for (auto st : state) {
    (void)st;
    auto ibuf = grape::serdes::InStream(obuf.data());
    auto deserializer = grape::serdes::Deserialiser(ibuf);
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
