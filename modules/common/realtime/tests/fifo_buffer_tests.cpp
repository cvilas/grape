//=================================================================================================
// Copyright (C) 2018 GRAPE Contributors
//=================================================================================================

#include <thread>

#include "catch2/catch_test_macros.hpp"
#include "grape/realtime/fifo_buffer.h"

namespace {

// NOLINTBEGIN(cert-err58-cpp,cppcoreguidelines-avoid-magic-numbers)

//-------------------------------------------------------------------------------------------------
TEST_CASE("Count should be zero on initialization", "[FIFOBuffer]") {
  constexpr grape::realtime::FIFOBuffer::Config CONFIG{ .frame_length = 64U, .num_frames = 10U };
  const auto buffer = grape::realtime::FIFOBuffer(CONFIG);
  REQUIRE(buffer.count() == 0);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Writes and reads are in FIFO order", "[FIFOBuffer]") {
  constexpr grape::realtime::FIFOBuffer::Config CONFIG{ .frame_length = 64U, .num_frames = 5U };
  grape::realtime::FIFOBuffer buffer(CONFIG);

  // write
  for (std::size_t i = 0; i < CONFIG.num_frames; ++i) {
    const auto test_data = std::vector<std::byte>(CONFIG.frame_length, static_cast<std::byte>(i));
    REQUIRE(buffer.visitToWrite([&test_data](std::span<std::byte> frame) {
      std::copy(test_data.begin(), test_data.end(), frame.begin());
    }));
    REQUIRE(buffer.count() == i + 1);
  }

  // read
  for (std::size_t i = 0; i < CONFIG.num_frames; ++i) {
    std::vector<std::byte> read_data(CONFIG.frame_length, static_cast<std::byte>(0xff));
    REQUIRE(buffer.visitToRead([&read_data](std::span<const std::byte> frame) {
      std::copy(frame.begin(), frame.end(), read_data.begin());
    }));
    REQUIRE(buffer.count() == CONFIG.num_frames - i - 1);
    REQUIRE(std::vector<std::byte>(CONFIG.frame_length, static_cast<std::byte>(i)) == read_data);
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Multiple threads can write", "[FIFOBuffer]") {
  constexpr grape::realtime::FIFOBuffer::Config CONFIG{ .frame_length = 64U, .num_frames = 5U };
  grape::realtime::FIFOBuffer buffer(CONFIG);

  static constexpr std::uint8_t NUM_THREADS = 4U;
  std::vector<std::thread> threads;

  for (std::uint8_t i = 0; i < NUM_THREADS; ++i) {
    threads.emplace_back([&buffer]() {
      REQUIRE(buffer.visitToWrite([&](std::span<std::byte>) {
        // write operation
      }));
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  REQUIRE(buffer.count() == NUM_THREADS);

  for (std::uint8_t i = 0; i < NUM_THREADS; ++i) {
    REQUIRE(buffer.visitToRead([&](std::span<const std::byte>) {
      // reading operation
    }));
  }

  REQUIRE(buffer.count() == 0);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Attempt to write to a full buffer should fail", "[FIFOBuffer]") {
  constexpr grape::realtime::FIFOBuffer::Config CONFIG{ .frame_length = 64U, .num_frames = 5U };
  grape::realtime::FIFOBuffer buffer(CONFIG);
  for (std::size_t i = 0; i < CONFIG.num_frames; ++i) {
    REQUIRE(buffer.visitToWrite([&](std::span<std::byte>) {
      // Writing to fill up the buffer
    }));
  }
  REQUIRE(buffer.count() == CONFIG.num_frames);
  REQUIRE_FALSE(buffer.visitToWrite([&](std::span<std::byte>) {
    // Attempting to write to a full buffer
  }));
  REQUIRE(buffer.count() == CONFIG.num_frames);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Attempt to read from an empty buffer should fail", "[FIFOBuffer]") {
  constexpr grape::realtime::FIFOBuffer::Config CONFIG{ .frame_length = 64U, .num_frames = 5U };
  grape::realtime::FIFOBuffer buffer(CONFIG);
  REQUIRE_FALSE(buffer.visitToRead([&](std::span<const std::byte>) {
    // Attempting to read from an empty buffer
  }));
  REQUIRE(buffer.count() == 0);
}

// NOLINTEND(cert-err58-cpp,cppcoreguidelines-avoid-magic-numbers)

}  // namespace
