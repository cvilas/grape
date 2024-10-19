//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <benchmark/benchmark.h>

#include "grape/serdes/serdes.h"
#include "grape/serdes/stream.h"

namespace {

//-------------------------------------------------------------------------------------------------
// Define a simple struct to serialize
struct Person {
  std::string name;
  std::int32_t id{};
  std::string email;
  std::vector<float> scores;
};

//-------------------------------------------------------------------------------------------------
auto generatePerson() -> Person {
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
  return {
    .name = "John Doe", .id = 25, .email = "john.doe@example.com", .scores = { 2.1f, 3.2f, 4.5f }
  };
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
}

//-------------------------------------------------------------------------------------------------
template <grape::serdes::WritableStream Stream>
[[nodiscard]] auto serialize(grape::serdes::Serialiser<Stream>& ser, const Person& p) -> bool {
  if (not ser.pack(p.name)) {
    return false;
  }
  if (not ser.pack(p.id)) {
    return false;
  }
  if (not ser.pack(p.email)) {
    return false;
  }
  if (not ser.pack(p.scores)) {
    return false;
  }
  return true;
}

//-------------------------------------------------------------------------------------------------
template <grape::serdes::ReadableStream Stream>
[[nodiscard]] auto deserialize(grape::serdes::Deserialiser<Stream>& des, Person& p) -> bool {
  if (not des.unpack(p.name)) {
    return false;
  }
  if (not des.unpack(p.id)) {
    return false;
  }
  if (not des.unpack(p.email)) {
    return false;
  }
  if (not des.unpack(p.scores)) {
    return false;
  }
  return true;
}

constexpr auto BUFFER_INIT_SIZE = 1024u;

//-------------------------------------------------------------------------------------------------
void bmSerialize(benchmark::State& state) {
  const auto p = generatePerson();
  auto buf = grape::serdes::OutStream<BUFFER_INIT_SIZE>();

  for (auto s : state) {
    (void)s;
    auto serializer = grape::serdes::Serialiser(buf);
    if (not serialize(serializer, p)) {
      throw std::runtime_error("Serialisation error");
    }

    benchmark::DoNotOptimize(buf.data());
    benchmark::ClobberMemory();
  }
}

//-------------------------------------------------------------------------------------------------
void bmDeserialize(benchmark::State& state) {
  const auto p = generatePerson();

  auto obuf = grape::serdes::OutStream<BUFFER_INIT_SIZE>();
  auto serializer = grape::serdes::Serialiser(obuf);
  if (not serialize(serializer, p)) {
    throw std::runtime_error("Serialisation error");
  }

  for (auto s : state) {
    (void)s;
    auto ibuf = grape::serdes::InStream({ obuf.data(), obuf.size() });
    auto deserializer = grape::serdes::Deserialiser(ibuf);
    Person deserialized_person;
    if (not deserialize(deserializer, deserialized_person)) {
      throw std::runtime_error("Deserialisation error");
    }
  }
}

// NOLINTBEGIN(cppcoreguidelines-owning-memory,cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables)
BENCHMARK(bmSerialize);
BENCHMARK(bmDeserialize);
//  NOLINTEND(cppcoreguidelines-owning-memory,cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables)

}  // namespace

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,cppcoreguidelines-pro-bounds-array-to-pointer-decay,modernize-use-trailing-return-type)
BENCHMARK_MAIN();
