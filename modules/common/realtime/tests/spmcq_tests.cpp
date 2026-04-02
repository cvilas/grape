//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <cstring>
#include <format>
#include <thread>

#include "catch2/catch_test_macros.hpp"
#include "grape/realtime/spmc_ring_buffer.h"
#include "grape/wall_clock.h"

namespace {

using Config = grape::spmc_ring_buffer::Config;
using Reader = grape::spmc_ring_buffer::Reader;
using Writer = grape::spmc_ring_buffer::Writer;

//-------------------------------------------------------------------------------------------------
auto uniqueName() -> std::string {
  return std::format("spmcq_test_{}", grape::WallClock::now().time_since_epoch().count());
}

//-------------------------------------------------------------------------------------------------
// Helpers to write/read a uint64_t value via the visitor interface.
void writeValue(Writer& writer, std::uint64_t value) {
  writer.visit([&value](std::span<std::byte> frame) -> bool {
    std::memcpy(frame.data(), &value, sizeof(value));
    return true;
  });
}

auto readValue(Reader& reader, Reader::Policy policy) -> std::pair<Reader::Status, std::uint64_t> {
  std::uint64_t value{};
  const auto status = reader.visit(
      [&value](std::span<const std::byte> frame) {
        std::memcpy(&value, frame.data(), sizeof(value));
        return true;
      },
      policy);
  return { status, value };
}

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

//-------------------------------------------------------------------------------------------------
TEST_CASE("Invalid config returns error", "[spmcq]") {
  REQUIRE_FALSE(Writer::create(uniqueName(), Config{ .frame_length = 0U, .num_frames = 1U }));
  REQUIRE_FALSE(Writer::create(uniqueName(), Config{ .frame_length = 1U, .num_frames = 0U }));
  REQUIRE_FALSE(Writer::create(uniqueName(), Config{ .frame_length = 0U, .num_frames = 0U }));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Read from empty buffer returns Empty", "[spmcq]") {
  const auto name = uniqueName();
  auto maybe_writer = Writer::create(name, Config{ .frame_length = 8U, .num_frames = 4U });
  REQUIRE(maybe_writer);
  auto maybe_reader = Reader::connect(name);
  REQUIRE(maybe_reader);
  auto& reader = maybe_reader.value();

  const auto [status, unused] = readValue(reader, Reader::Policy::Next);
  REQUIRE(status == Reader::Status::Empty);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Single write and read round-trips data", "[spmcq]") {
  const auto name = uniqueName();
  auto maybe_writer = Writer::create(name, Config{ .frame_length = 8U, .num_frames = 4U });
  REQUIRE(maybe_writer);
  auto& writer = maybe_writer.value();
  auto maybe_reader = Reader::connect(name);
  REQUIRE(maybe_reader);
  auto& reader = maybe_reader.value();

  static constexpr auto EXPECTED = std::uint64_t{ 0xDEADBEEF };
  writeValue(writer, EXPECTED);
  const auto [status, value] = readValue(reader, Reader::Policy::Next);
  REQUIRE(status == Reader::Status::Ok);
  REQUIRE(value == EXPECTED);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Reads are in FIFO order", "[spmcq]") {
  static constexpr auto NUM_FRAMES = 8UZ;
  const auto name = uniqueName();
  auto maybe_writer = Writer::create(name, Config{ .frame_length = 8U, .num_frames = NUM_FRAMES });
  REQUIRE(maybe_writer);
  auto& writer = maybe_writer.value();
  auto maybe_reader = Reader::connect(name);
  REQUIRE(maybe_reader);
  auto& reader = maybe_reader.value();

  for (std::uint64_t i = 0; i < NUM_FRAMES - 1; ++i) {
    writeValue(writer, i);
  }

  for (std::uint64_t i = 0; i < NUM_FRAMES - 1; ++i) {
    const auto [status, value] = readValue(reader, Reader::Policy::Next);
    REQUIRE(status == Reader::Status::Ok);
    REQUIRE(value == i);
  }

  // Buffer is now drained
  const auto [status, unused] = readValue(reader, Reader::Policy::Next);
  REQUIRE(status == Reader::Status::Empty);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Write to full buffer never fails (ring semantics)", "[spmcq]") {
  static constexpr auto CAPACITY = 4UZ;
  const auto name = uniqueName();
  auto maybe_writer = Writer::create(name, Config{ .frame_length = 8U, .num_frames = CAPACITY });
  REQUIRE(maybe_writer);
  auto& writer = maybe_writer.value();

  // Writing more than capacity must always succeed; writer laps silently.
  for (std::uint64_t i = 0; i < CAPACITY * 3; ++i) {
    REQUIRE_NOTHROW(writeValue(writer, i));
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Writer lapping reader returns Dropped", "[spmcq]") {
  static constexpr auto CAPACITY = 4UZ;
  const auto name = uniqueName();
  auto maybe_writer = Writer::create(name, Config{ .frame_length = 8U, .num_frames = CAPACITY });
  REQUIRE(maybe_writer);
  auto& writer = maybe_writer.value();
  auto maybe_reader = Reader::connect(name);
  REQUIRE(maybe_reader);
  auto& reader = maybe_reader.value();

  // Fill past capacity so reader is fully lapped.
  for (std::uint64_t i = 0; i < CAPACITY + 1; ++i) {
    writeValue(writer, i);
  }

  const auto [status, unused] = readValue(reader, Reader::Policy::Next);
  REQUIRE(status == Reader::Status::Dropped);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("After Dropped, reader is at write_count and reports Empty", "[spmcq]") {
  static constexpr auto CAPACITY = 4UZ;
  const auto name = uniqueName();
  auto maybe_writer = Writer::create(name, Config{ .frame_length = 8U, .num_frames = CAPACITY });
  REQUIRE(maybe_writer);
  auto& writer = maybe_writer.value();
  auto maybe_reader = Reader::connect(name);
  REQUIRE(maybe_reader);
  auto& reader = maybe_reader.value();

  for (std::uint64_t i = 0; i < CAPACITY + 1; ++i) {
    writeValue(writer, i);
  }

  // Consume the Dropped status
  REQUIRE(readValue(reader, Reader::Policy::Next).first == Reader::Status::Dropped);
  // Reader is now at write_count; no further frames to read
  REQUIRE(readValue(reader, Reader::Policy::Next).first == Reader::Status::Empty);

  // New write is immediately readable
  static constexpr auto NEW_VALUE = std::uint64_t{ 99 };
  writeValue(writer, NEW_VALUE);
  const auto [status, value] = readValue(reader, Reader::Policy::Next);
  REQUIRE(status == Reader::Status::Ok);
  REQUIRE(value == NEW_VALUE);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Only one writer allowed at a time", "[spmcq]") {
  const auto name = uniqueName();
  auto maybe_writer = Writer::create(name, Config{ .frame_length = 8U, .num_frames = 4U });
  REQUIRE(maybe_writer);

  // Second writer with the same name must fail.
  const auto maybe_dup = Writer::create(name, Config{ .frame_length = 8U, .num_frames = 4U });
  REQUIRE_FALSE(maybe_dup);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Connect to non-existent buffer returns error", "[spmcq]") {
  static constexpr auto UNKNOWN = "no_such_spmcq_buffer_xyz";
  REQUIRE_FALSE(Reader::connect(UNKNOWN));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("exists() reflects writer lifecycle", "[spmcq]") {
  const auto name = uniqueName();
  REQUIRE_FALSE(Reader::exists(name));
  {
    auto maybe_writer = Writer::create(name, Config{ .frame_length = 8U, .num_frames = 4U });
    REQUIRE(maybe_writer);
    REQUIRE(Reader::exists(name));
  }  // Writer destructor removes SHM
  REQUIRE_FALSE(Reader::exists(name));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Reader config matches writer config", "[spmcq]") {
  const auto name = uniqueName();
  static constexpr auto CFG = Config{ .frame_length = 16U, .num_frames = 7U };
  auto maybe_writer = Writer::create(name, CFG);
  REQUIRE(maybe_writer);
  auto maybe_reader = Reader::connect(name);
  REQUIRE(maybe_reader);

  const auto cfg = maybe_reader.value().config();
  REQUIRE(cfg.frame_length == CFG.frame_length);
  REQUIRE(cfg.num_frames == CFG.num_frames);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Multiple readers independently track their own read position", "[spmcq]") {
  static constexpr auto NUM_FRAMES = 4UZ;
  const auto name = uniqueName();
  auto maybe_writer = Writer::create(name, Config{ .frame_length = 8U, .num_frames = NUM_FRAMES });
  REQUIRE(maybe_writer);
  auto& writer = maybe_writer.value();
  auto maybe_reader_a = Reader::connect(name);
  REQUIRE(maybe_reader_a);
  auto maybe_reader_b = Reader::connect(name);
  REQUIRE(maybe_reader_b);
  auto& reader_a = maybe_reader_a.value();
  auto& reader_b = maybe_reader_b.value();

  writeValue(writer, 10U);
  writeValue(writer, 20U);

  // Both readers independently read the same two frames in order.
  const auto [sa1, va1] = readValue(reader_a, Reader::Policy::Next);
  const auto [sb1, vb1] = readValue(reader_b, Reader::Policy::Next);
  REQUIRE(sa1 == Reader::Status::Ok);
  REQUIRE(sb1 == Reader::Status::Ok);
  REQUIRE(va1 == 10U);
  REQUIRE(vb1 == 10U);

  const auto [sa2, va2] = readValue(reader_a, Reader::Policy::Next);
  const auto [sb2, vb2] = readValue(reader_b, Reader::Policy::Next);
  REQUIRE(sa2 == Reader::Status::Ok);
  REQUIRE(sb2 == Reader::Status::Ok);
  REQUIRE(va2 == 20U);
  REQUIRE(vb2 == 20U);

  // Reader A drained, reader B continues from its own position (also drained).
  REQUIRE(readValue(reader_a, Reader::Policy::Next).first == Reader::Status::Empty);
  REQUIRE(readValue(reader_b, Reader::Policy::Next).first == Reader::Status::Empty);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Concurrent writer and reader", "[spmcq]") {
  static constexpr auto NUM_WRITES = 1000UZ;
  static constexpr auto CAPACITY = NUM_WRITES + 1UZ;  // large enough to avoid lapping
  const auto name = uniqueName();
  auto maybe_writer = Writer::create(name, Config{ .frame_length = 8U, .num_frames = CAPACITY });
  REQUIRE(maybe_writer);
  auto maybe_reader = Reader::connect(name);
  REQUIRE(maybe_reader);
  auto& writer = maybe_writer.value();
  auto& reader = maybe_reader.value();

  std::atomic<bool> done{ false };

  std::thread producer([&writer, &done]() {
    for (std::uint64_t i = 0; i < NUM_WRITES; ++i) {
      writeValue(writer, i);
    }
    done.store(true, std::memory_order_release);
  });

  std::uint64_t reads = 0;
  while (reads < NUM_WRITES) {
    const auto [status, value] = readValue(reader, Reader::Policy::Next);
    if (status == Reader::Status::Ok) {
      ++reads;
    }
  }
  REQUIRE(reads == NUM_WRITES);

  producer.join();
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Writer does not advance if write function returns false", "[spmcq]") {
  const auto name = uniqueName();
  auto maybe_writer = Writer::create(name, Config{ .frame_length = 8U, .num_frames = 4U });
  REQUIRE(maybe_writer);
  auto& writer = maybe_writer.value();
  auto maybe_reader = Reader::connect(name);
  REQUIRE(maybe_reader);
  auto& reader = maybe_reader.value();

  // Write a valid frame
  writeValue(writer, 123U);
  REQUIRE(readValue(reader, Reader::Policy::Next).first == Reader::Status::Ok);

  // Attempt to write a frame but return false to abort; write count should not advance
  writer.visit([](std::span<std::byte>) { return false; });
  REQUIRE(readValue(reader, Reader::Policy::Next).first == Reader::Status::Empty);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Reader does not advance if read function returns false", "[spmcq]") {
  const auto name = uniqueName();
  auto maybe_writer = Writer::create(name, Config{ .frame_length = 8U, .num_frames = 4U });
  REQUIRE(maybe_writer);
  auto& writer = maybe_writer.value();
  auto maybe_reader = Reader::connect(name);
  REQUIRE(maybe_reader);
  auto& reader = maybe_reader.value();

  // Write a valid frame
  static constexpr auto TEST_VALUE = std::uint64_t{ 123 };
  writeValue(writer, TEST_VALUE);

  // Attempt to read a frame but return false to abort; read count should not advance
  REQUIRE(reader.visit([](std::span<const std::byte>) { return false; }) ==
          Reader::Status::Canceled);

  // next successful read returns the frame
  auto [status, value] = readValue(reader, Reader::Policy::Next);
  REQUIRE(status == Reader::Status::Ok);
  REQUIRE(value == TEST_VALUE);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Reader policy Latest skips to latest frame", "[spmcq]") {
  static constexpr auto NUM_FRAMES = 4UZ;
  const auto name = uniqueName();
  auto maybe_writer = Writer::create(name, Config{ .frame_length = 8U, .num_frames = NUM_FRAMES });
  REQUIRE(maybe_writer);
  auto& writer = maybe_writer.value();
  auto maybe_reader = Reader::connect(name);
  REQUIRE(maybe_reader);
  auto& reader = maybe_reader.value();

  writeValue(writer, 1UL);
  writeValue(writer, 2UL);
  writeValue(writer, 3UL);

  // first read should be 1U, latest read should be 3U
  const auto [status1, value1] = readValue(reader, Reader::Policy::Next);
  REQUIRE(status1 == Reader::Status::Ok);
  REQUIRE(value1 == 1UL);

  const auto [status2, value2] = readValue(reader, Reader::Policy::Latest);
  REQUIRE(status2 == Reader::Status::Ok);
  REQUIRE(value2 == 3UL);
}
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace
