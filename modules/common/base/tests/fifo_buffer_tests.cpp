//=================================================================================================
// Copyright (C) 2018 GRAPE Contributors
//=================================================================================================

#include <stdexcept>

#include "catch2/catch_test_macros.hpp"
#include "grape/fifo_buffer.h"

namespace {

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

//-------------------------------------------------------------------------------------------------
TEST_CASE("Count should be zero on initialization", "[FIFOBuffer]") {
  constexpr grape::FIFOBuffer::Config CONFIG{ .frame_length = 64U, .num_frames = 10U };
  const auto buffer = grape::FIFOBuffer(CONFIG);
  REQUIRE(buffer.count() == 0);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Writes and reads are in FIFO order", "[FIFOBuffer]") {
  constexpr grape::FIFOBuffer::Config CONFIG{ .frame_length = 64U, .num_frames = 5U };
  grape::FIFOBuffer buffer(CONFIG);

  // write
  for (std::size_t i = 0; i < CONFIG.num_frames; ++i) {
    const auto test_data = std::vector<std::byte>(CONFIG.frame_length, static_cast<std::byte>(i));
    REQUIRE(buffer.visitToWrite([&test_data](std::span<std::byte> frame) -> void {
      std::ranges::copy(test_data, frame.begin());
    }));
    REQUIRE(buffer.count() == i + 1);
  }

  // read
  for (std::size_t i = 0; i < CONFIG.num_frames; ++i) {
    std::vector<std::byte> read_data(CONFIG.frame_length, static_cast<std::byte>(0xff));
    REQUIRE(buffer.visitToRead([&read_data](std::span<const std::byte> frame) -> void {
      std::ranges::copy(frame, read_data.begin());
    }));
    REQUIRE(buffer.count() == CONFIG.num_frames - i - 1);
    REQUIRE(std::vector<std::byte>(CONFIG.frame_length, static_cast<std::byte>(i)) == read_data);
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Attempt to write to a full buffer should fail", "[FIFOBuffer]") {
  constexpr grape::FIFOBuffer::Config CONFIG{ .frame_length = 64U, .num_frames = 5U };
  grape::FIFOBuffer buffer(CONFIG);
  for (std::size_t i = 0; i < CONFIG.num_frames; ++i) {
    REQUIRE(buffer.visitToWrite([&](std::span<std::byte>) -> void {
      // Writing to fill up the buffer
    }));
  }
  REQUIRE(buffer.count() == CONFIG.num_frames);
  REQUIRE_FALSE(buffer.visitToWrite([&](std::span<std::byte>) -> void {
    // Attempting to write to a full buffer
  }));
  REQUIRE(buffer.count() == CONFIG.num_frames);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Attempt to read from an empty buffer should fail", "[FIFOBuffer]") {
  constexpr grape::FIFOBuffer::Config CONFIG{ .frame_length = 64U, .num_frames = 5U };
  grape::FIFOBuffer buffer(CONFIG);
  REQUIRE_FALSE(buffer.visitToRead([&](std::span<const std::byte>) -> void {
    // Attempting to read from an empty buffer
  }));
  REQUIRE(buffer.count() == 0);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("A failed read keeps the frame available for retry", "[FIFOBuffer]") {
  constexpr grape::FIFOBuffer::Config CONFIG{ .frame_length = 1U, .num_frames = 1U };
  grape::FIFOBuffer buffer(CONFIG);
  REQUIRE(buffer.visitToWrite(
      [](std::span<std::byte> frame) -> void { frame.front() = std::byte{ 42 }; }));

  REQUIRE_THROWS_AS(buffer.visitToRead([](std::span<const std::byte>) -> void {
    throw std::runtime_error("read");
  }),
                    std::runtime_error);
  REQUIRE(buffer.count() == 1U);

  auto value = std::byte{};
  REQUIRE(buffer.visitToRead(
      [&value](std::span<const std::byte> frame) -> void { value = frame.front(); }));
  REQUIRE(value == std::byte{ 42 });
  REQUIRE(buffer.count() == 0U);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace
