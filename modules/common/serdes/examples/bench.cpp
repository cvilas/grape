#include <cstdlib>
#include <string>
#include <vector>

#include <benchmark/benchmark.h>
#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>
#include <fastcdr/FastCdr.h>
#include <flatbuffers/flexbuffers.h>
#include <msgpack.hpp>

//-------------------------------------------------------------------------------------------------
// Define a simple struct to serialize
struct Person {
  std::string name;
  int32_t id{};
  std::string email;
  std::vector<float> scores;
};

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
  namespace adaptor {

  template <>
  struct pack<Person> {
    template <typename Stream>
    auto operator()(msgpack::packer<Stream>& o, Person const& v) const -> msgpack::packer<Stream>& {
      o.pack_array(4);
      o.pack(v.name);
      o.pack(v.id);
      o.pack(v.email);
      o.pack(v.scores);
      return o;
    }
  };

  template <>
  struct convert<Person> {
    auto operator()(msgpack::object const& o, Person& v) const -> msgpack::object const& {
      if (o.type != msgpack::type::ARRAY) {
        throw msgpack::type_error();
      }
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
      const auto& arr = o.via.array;
      if (arr.size != 4) {
        throw msgpack::type_error();
      }
      // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      arr.ptr[0].convert(v.name);
      arr.ptr[1].convert(v.id);
      arr.ptr[2].convert(v.email);
      arr.ptr[3].convert(v.scores);
      // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

      return o;
    }
  };

  }  // namespace adaptor
}  // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
}  // namespace msgpack

namespace {

//-------------------------------------------------------------------------------------------------
auto generatePerson() -> Person {
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
  return Person{
    .name = "John Doe", .id = 25, .email = "john.doe@example.com", .scores = { 2.1f, 3.2f, 4.5f }
  };
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
}

constexpr auto BUFFER_INIT_SIZE = 1024u;

//-------------------------------------------------------------------------------------------------
void bmMsgpackSerialize(benchmark::State& state) {
  const auto p = generatePerson();
  for (auto s : state) {
    (void)s;
    msgpack::sbuffer ss(BUFFER_INIT_SIZE);
    msgpack::pack(ss, p);
    benchmark::DoNotOptimize(ss);
    benchmark::ClobberMemory();
  }
}

//-------------------------------------------------------------------------------------------------
void bmMsgpackDeserialize(benchmark::State& state) {
  const auto p = generatePerson();
  msgpack::sbuffer ss;
  msgpack::pack(ss, p);

  for (auto s : state) {
    (void)s;
    Person pd;
    const auto oh = msgpack::unpack(ss.data(), ss.size());
    oh.get().convert(pd);

    benchmark::DoNotOptimize(pd);
    benchmark::ClobberMemory();
  }
}

//-------------------------------------------------------------------------------------------------
void serialize(eprosima::fastcdr::Cdr& cdr, const Person& p) {
  cdr << p.name;
  cdr << p.id;
  cdr << p.email;
  cdr << p.scores;
}

//-------------------------------------------------------------------------------------------------
void deserialize(eprosima::fastcdr::Cdr& cdr, Person& p) {
  cdr >> p.name;
  cdr >> p.id;
  cdr >> p.email;
  cdr >> p.scores;
}

//-------------------------------------------------------------------------------------------------
void bmCDRSerialize(benchmark::State& state) {
  const auto p = generatePerson();
  eprosima::fastcdr::FastBuffer buffer;
  buffer.reserve(BUFFER_INIT_SIZE);
  for (auto s : state) {
    (void)s;
    // Serialize the data
    eprosima::fastcdr::Cdr serializer(buffer);
    serialize(serializer, p);

    auto* v = serializer.get_buffer_pointer();
    benchmark::DoNotOptimize(v);
    benchmark::ClobberMemory();
  }
}

//-------------------------------------------------------------------------------------------------
void bmCDRDeserialize(benchmark::State& state) {
  // Create a serialized buffer to use in deserialization
  const auto p = generatePerson();

  eprosima::fastcdr::FastBuffer buffer;
  eprosima::fastcdr::Cdr serializer(buffer);
  serialize(serializer, p);  // Serialize the person into buffer first

  for (auto s : state) {
    (void)s;
    // Reset position to the beginning for deserialization
    eprosima::fastcdr::Cdr deserializer(buffer);
    Person deserialized_person;
    deserialize(deserializer, deserialized_person);
  }
}

//-------------------------------------------------------------------------------------------------
void serialize(eprosima::fastcdr::FastCdr& cdr, const Person& p) {
  cdr << p.name;
  cdr << p.id;
  cdr << p.email;
  cdr << p.scores;
}

//-------------------------------------------------------------------------------------------------
void deserialize(eprosima::fastcdr::FastCdr& cdr, Person& p) {
  cdr >> p.name;
  cdr >> p.id;
  cdr >> p.email;
  cdr >> p.scores;
}

//-------------------------------------------------------------------------------------------------
void bmFastCDRSerialize(benchmark::State& state) {
  const auto p = generatePerson();
  eprosima::fastcdr::FastBuffer buffer;
  buffer.reserve(BUFFER_INIT_SIZE);
  for (auto s : state) {
    (void)s;
    // Serialize the data
    eprosima::fastcdr::FastCdr serializer(buffer);
    serialize(serializer, p);

    auto* v = serializer.get_current_position();
    benchmark::DoNotOptimize(v);
    benchmark::ClobberMemory();
  }
}

//-------------------------------------------------------------------------------------------------
void bmFastCDRDeserialize(benchmark::State& state) {
  // Create a serialized buffer to use in deserialization
  const auto p = generatePerson();

  eprosima::fastcdr::FastBuffer buffer;
  eprosima::fastcdr::FastCdr serializer(buffer);
  serialize(serializer, p);  // Serialize the person into buffer first

  for (auto s : state) {
    (void)s;
    // Reset position to the beginning for deserialization
    eprosima::fastcdr::FastCdr deserializer(buffer);
    Person deserialized_person;
    deserialize(deserializer, deserialized_person);
  }
}

//-------------------------------------------------------------------------------------------------
void serialize(flexbuffers::Builder& fbb, const Person& p) {
  fbb.Vector([&]() {
    fbb.String(p.name);
    fbb.Int(p.id);
    fbb.String(p.email);
    fbb.Vector(p.scores);
  });
  fbb.Finish();
}

//-------------------------------------------------------------------------------------------------
void deserialize(flexbuffers::Builder& fbb, Person& pd) {
  const auto root = flexbuffers::GetRoot(fbb.GetBuffer()).AsVector();
  pd.name = root[0].AsString().str();
  pd.id = root[1].AsInt32();
  pd.email = root[2].AsString().str();
  const auto scores = root[3].AsTypedVector();
  pd.scores.resize(scores.size());
  for (size_t i = 0; i < scores.size(); ++i) {
    pd.scores[i] = scores[i].AsFloat();
  }
}

//-------------------------------------------------------------------------------------------------
void bmFlexBufferSerialize(benchmark::State& state) {
  const auto p = generatePerson();
  for (auto s : state) {
    (void)s;
    auto fbb = flexbuffers::Builder{ BUFFER_INIT_SIZE };
    serialize(fbb, p);
    fbb.Finish();

    auto v = fbb.GetBuffer();
    benchmark::DoNotOptimize(v);
    benchmark::ClobberMemory();
  }
}

//-------------------------------------------------------------------------------------------------
void bmFlexBufferDeserialize(benchmark::State& state) {
  const auto p = generatePerson();
  auto fbb = flexbuffers::Builder{};
  serialize(fbb, p);
  fbb.Finish();

  for (auto s : state) {
    (void)s;
    Person pd;
    deserialize(fbb, pd);
    benchmark::DoNotOptimize(pd);
    benchmark::ClobberMemory();
  }
}

// NOLINTBEGIN(cppcoreguidelines-owning-memory,cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables)
BENCHMARK(bmFastCDRSerialize);
BENCHMARK(bmFastCDRDeserialize);
BENCHMARK(bmCDRSerialize);
BENCHMARK(bmCDRDeserialize);
BENCHMARK(bmMsgpackSerialize);
BENCHMARK(bmMsgpackDeserialize);
BENCHMARK(bmFlexBufferSerialize);
BENCHMARK(bmFlexBufferDeserialize);
// NOLINTEND(cppcoreguidelines-owning-memory,cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables)

}  // namespace

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,cppcoreguidelines-pro-bounds-array-to-pointer-decay,modernize-use-trailing-return-type)
BENCHMARK_MAIN();