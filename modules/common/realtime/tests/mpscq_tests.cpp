//=================================================================================================
// Copyright (C) 2018 GRAPE Contributors
//=================================================================================================

#include <thread>

#include "catch2/catch_test_macros.hpp"
#include "grape/realtime/mpsc_queue.h"

namespace {

//-------------------------------------------------------------------------------------------------
TEST_CASE("Basline single producer and single consumer case", "[mpsc_queue]") {
  static constexpr std::size_t CAPACITY = 3;
  grape::realtime::MPSCQueue<int> queue(CAPACITY);

  REQUIRE(queue.tryPush(1));
  REQUIRE(queue.tryPush(2));
  REQUIRE(queue.tryPush(3));

  REQUIRE(queue.count() == 3u);

  // NOLINTBEGIN(bugprone-unchecked-optional-access)
  REQUIRE(queue.tryPop().value() == 1);
  REQUIRE(queue.tryPop().value() == 2);
  REQUIRE(queue.tryPop().value() == 3);
  // NOLINTEND(bugprone-unchecked-optional-access)

  REQUIRE(queue.count() == 0u);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Multiple producers should be able to push", "[mpsc_queue]") {
  static constexpr std::size_t CAPACITY = 100;
  grape::realtime::MPSCQueue<int> queue(CAPACITY);

  static constexpr std::size_t NUM_PRODUCERS = 5;
  static constexpr std::size_t NUM_ITEMS_PER_PRODUCER = 10;

  std::vector<std::thread> producers;
  producers.reserve(NUM_PRODUCERS);
  for (std::size_t i = 0; i < NUM_PRODUCERS; ++i) {
    producers.emplace_back([&queue, i]() {
      for (std::size_t j = 0; j < NUM_ITEMS_PER_PRODUCER; ++j) {
        std::ignore = queue.tryPush(static_cast<int>((i * NUM_ITEMS_PER_PRODUCER) + j));
      }
    });
  }

  for (auto& producer : producers) {
    producer.join();
  }
  static constexpr auto EXPECTED_COUNT = NUM_PRODUCERS * NUM_ITEMS_PER_PRODUCER;
  REQUIRE(queue.count() == EXPECTED_COUNT);

  std::vector<int> popped_items;
  popped_items.reserve(EXPECTED_COUNT);
  while (auto item = queue.tryPop()) {
    if (item.has_value()) {
      popped_items.push_back(item.value());
    }
  }

  REQUIRE(popped_items.size() == EXPECTED_COUNT);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Pushing into full queue should return false", "[mpsc_queue]") {
  static constexpr std::size_t CAPACITY = 2;
  grape::realtime::MPSCQueue<int> queue(CAPACITY);

  REQUIRE(queue.tryPush(1));
  REQUIRE(queue.tryPush(2));

  REQUIRE_FALSE(queue.tryPush(3));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Popping empty queue should return false", "[mpsc_queue]") {
  static constexpr std::size_t CAPACITY = 3;
  grape::realtime::MPSCQueue<int> queue(CAPACITY);

  REQUIRE_FALSE(queue.tryPop().has_value());
}

}  // namespace
