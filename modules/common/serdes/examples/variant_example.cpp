//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <print>
#include <variant>

#include "grape/serdes/serdes.h"
#include "grape/serdes/stream.h"

namespace {

constexpr auto BUF_SIZE = 1024U;
using OutStream = grape::serdes::OutStream<BUF_SIZE>;
using InStream = grape::serdes::InStream;
using Serialiser = grape::serdes::Serialiser<OutStream>;
using Deserialiser = grape::serdes::Deserialiser<InStream>;

//-------------------------------------------------------------------------------------------------
struct Sit {
  float speed{ 0.0F };
};

//-------------------------------------------------------------------------------------------------
struct Stand {
  float speed{ 0.0F };
};

//-------------------------------------------------------------------------------------------------
struct Walk {
  float forward_speed{ 0.0F };
  float lateral_speed{ 0.0F };
  float turn_speed{ 0.0F };
};

//-------------------------------------------------------------------------------------------------
[[nodiscard]] auto serialise(Serialiser& ser, const Sit& st) -> bool {
  return ser.pack(st.speed);
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] auto deserialise(Deserialiser& des, Sit& st) -> bool {
  return des.unpack(st.speed);
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] auto serialise(Serialiser& ser, const Stand& st) -> bool {
  return ser.pack(st.speed);
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] auto deserialise(Deserialiser& des, Stand& st) -> bool {
  return des.unpack(st.speed);
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] auto serialise(Serialiser& ser, const Walk& st) -> bool {
  return ser.pack(st.forward_speed) and ser.pack(st.lateral_speed) and ser.pack(st.turn_speed);
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] auto deserialise(Deserialiser& des, Walk& st) -> bool {
  return des.unpack(st.forward_speed) and des.unpack(st.lateral_speed) and
         des.unpack(st.turn_speed);
}

//-------------------------------------------------------------------------------------------------
struct VisitCommand {
  void operator()(const Sit& ev) {
    std::println("[Sit] speed={}", ev.speed);
  }
  void operator()(const Stand& ev) {
    std::println("[Stand] speed={}", ev.speed);
  }
  void operator()(const Walk& ev) {
    std::println("[Walk] forward_speed={}, lateral_speed={}, turn_speed={}", ev.forward_speed,
                 ev.lateral_speed, ev.turn_speed);
  }
};
}  // namespace

//=================================================================================================
/// Demonstrates serialisation and deserialisation of a std::variant
/// Custom data types must satisfy the Serialisable and Deserialisable concepts
auto main() -> int {
  try {
    using Command = std::variant<Sit, Stand, Walk>;

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    const auto cmd =
        Command{ Walk{ .forward_speed = .0F, .lateral_speed = .5F, .turn_speed = 0.2F } };

    // serialise
    auto ostream = OutStream();
    auto serialiser = Serialiser(ostream);
    if (not serialiser.pack(cmd)) {
      std::println("Serialisation error. Exiting");
      return EXIT_FAILURE;
    }

    // deserialise
    auto cmd2 = Command{};
    auto istream = InStream(ostream.data());
    auto deserialiser = Deserialiser(istream);
    if (not deserialiser.unpack(cmd2)) {
      std::println("Deserialisation error. Exiting");
      return EXIT_FAILURE;
    }

    std::visit(VisitCommand{}, cmd2);

    return EXIT_SUCCESS;
  } catch (const std::exception& e) {
    std::ignore = std::fputs(e.what(), stderr);
    return EXIT_FAILURE;
  }
}
