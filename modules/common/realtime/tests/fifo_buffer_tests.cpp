//=================================================================================================
// Copyright (C) 2018 GRAPE Contributors
//=================================================================================================

#include <thread>

#include "catch2/catch_test_macros.hpp"
#include "grape/realtime/fifo_buffer.h"

namespace grape::realtime::tests {

// NOLINTBEGIN(cert-err58-cpp,cppcoreguidelines-avoid-magic-numbers)

//-------------------------------------------------------------------------------------------------
TEST_CASE("Count should be zero on initialization", "[FIFOBuffer]") {
  grape::realtime::FIFOBuffer::Options options{ .frame_length = 64U, .num_frames = 10U };
  grape::realtime::FIFOBuffer buffer(options);
  REQUIRE((buffer.count() == 0));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Writes and reads are in FIFO order", "[FIFOBuffer]") {
  grape::realtime::FIFOBuffer::Options options{ .frame_length = 64U, .num_frames = 5U };
  grape::realtime::FIFOBuffer buffer(options);

  // write
  for (std::size_t i = 0; i < options.num_frames; ++i) {
    const auto test_data = std::vector<std::byte>(options.frame_length, static_cast<std::byte>(i));
    REQUIRE(buffer.visitToWrite([&test_data](std::span<std::byte> frame) {
      std::copy(test_data.begin(), test_data.end(), frame.begin());
    }));
    REQUIRE((buffer.count() == i + 1));
  }

  // read
  for (std::size_t i = 0; i < options.num_frames; ++i) {
    std::vector<std::byte> read_data(options.frame_length, static_cast<std::byte>(0xff));
    REQUIRE(buffer.visitToRead([&read_data](std::span<const std::byte> frame) {
      std::copy(frame.begin(), frame.end(), read_data.begin());
    }));
    REQUIRE((buffer.count() == options.num_frames - i - 1));
    REQUIRE((std::vector<std::byte>(options.frame_length, static_cast<std::byte>(i)) == read_data));
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Multiple threads can write", "[FIFOBuffer]") {
  grape::realtime::FIFOBuffer::Options options{ .frame_length = 64U, .num_frames = 5U };
  grape::realtime::FIFOBuffer buffer(options);

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

  REQUIRE((buffer.count() == NUM_THREADS));

  for (std::uint8_t i = 0; i < NUM_THREADS; ++i) {
    REQUIRE(buffer.visitToRead([&](std::span<const std::byte>) {
      // reading operation
    }));
  }

  REQUIRE((buffer.count() == 0));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Attempt to write to a full buffer should fail", "[FIFOBuffer]") {
  grape::realtime::FIFOBuffer::Options options{ .frame_length = 64U, .num_frames = 5U };
  grape::realtime::FIFOBuffer buffer(options);
  for (std::size_t i = 0; i < options.num_frames; ++i) {
    REQUIRE(buffer.visitToWrite([&](std::span<std::byte>) {
      // Writing to fill up the buffer
    }));
  }
  REQUIRE((buffer.count() == options.num_frames));
  REQUIRE_FALSE(buffer.visitToWrite([&](std::span<std::byte>) {
    // Attempting to write to a full buffer
  }));
  REQUIRE((buffer.count() == options.num_frames));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Attempt to read from an empty buffer should fail", "[FIFOBuffer]") {
  grape::realtime::FIFOBuffer::Options options{ .frame_length = 64U, .num_frames = 5U };
  grape::realtime::FIFOBuffer buffer(options);
  REQUIRE_FALSE(buffer.visitToRead([&](std::span<const std::byte>) {
    // Attempting to read from an empty buffer
  }));
  REQUIRE((buffer.count() == 0));
}

// NOLINTEND(cert-err58-cpp,cppcoreguidelines-avoid-magic-numbers)

}  // namespace grape::realtime::tests
