//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <stdexcept>

#include <benchmark/benchmark.h>

#include "advanced_example.h"
#include "grape/serdes/stream.h"

namespace {

constexpr auto BUFFER_INIT_SIZE = 1024u;

//-------------------------------------------------------------------------------------------------
void bmSerialize(benchmark::State& state) {
  const auto p = PoseStamped();
  auto buf = grape::serdes::OutStream<BUFFER_INIT_SIZE>();

  for (auto s : state) {
    (void)s;
    auto serializer = grape::serdes::Serialiser(buf);
    if (not pack(serializer, p)) {
      throw std::runtime_error("Serialisation error");
    }

    benchmark::DoNotOptimize(buf.data());
    benchmark::ClobberMemory();
  }
}

//-------------------------------------------------------------------------------------------------
void bmDeserialize(benchmark::State& state) {
  const auto p = PoseStamped();
  auto obuf = grape::serdes::OutStream<BUFFER_INIT_SIZE>();
  auto serializer = grape::serdes::Serialiser(obuf);
  if (not pack(serializer, p)) {
    throw std::runtime_error("Serialisation error");
  }

  for (auto s : state) {
    (void)s;
    auto ibuf = grape::serdes::InStream({ obuf.data(), obuf.size() });
    auto deserializer = grape::serdes::Deserialiser(ibuf);
    PoseStamped deserialized_pose;
    if (not unpack(deserializer, deserialized_pose)) {
      throw std::runtime_error("Deserialisation error");
    }

    benchmark::DoNotOptimize(deserialized_pose);
    benchmark::ClobberMemory();
  }
}

// NOLINTBEGIN(cppcoreguidelines-owning-memory,cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables)
BENCHMARK(bmSerialize);
BENCHMARK(bmDeserialize);
//   NOLINTEND(cppcoreguidelines-owning-memory,cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables)

}  // namespace

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,cppcoreguidelines-pro-bounds-array-to-pointer-decay,modernize-use-trailing-return-type)
BENCHMARK_MAIN();
