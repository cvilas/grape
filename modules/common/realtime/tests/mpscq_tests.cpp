//=================================================================================================
// Copyright (C) 2018 GRAPE Contributors
//=================================================================================================

#include <stdexcept>
#include <thread>
#include <utility>

#include "catch2/catch_test_macros.hpp"
#include "grape/realtime/mpsc_queue.h"

namespace {

// NOLINTBEGIN(cppcoreguidelines-special-member-functions, bugprone-exception-escape,
//             cppcoreguidelines-noexcept-move-operations)
struct ThrowOnMove {
  int value{};
  static inline auto throw_on_move = false;

  ThrowOnMove() = default;
  explicit ThrowOnMove(int val) : value(val) {
  }
  ~ThrowOnMove() = default;
  ThrowOnMove(const ThrowOnMove&) = delete;
  auto operator=(const ThrowOnMove&) -> ThrowOnMove& = delete;

  ThrowOnMove(ThrowOnMove&& other) {
    if (std::exchange(throw_on_move, false)) {
      throw std::runtime_error("move");
    }
    value = std::exchange(other.value, 0);
  }

  auto operator=(ThrowOnMove&& other) -> ThrowOnMove& {
    if (std::exchange(throw_on_move, false)) {
      throw std::runtime_error("move");
    }
    value = std::exchange(other.value, 0);
    return *this;
  }
};
// NOLINTEND(cppcoreguidelines-special-member-functions, bugprone-exception-escape,
//           cppcoreguidelines-noexcept-move-operations)

//-------------------------------------------------------------------------------------------------
TEST_CASE("Baseline single producer and single consumer case", "[mpsc_queue]") {
  static constexpr std::size_t CAPACITY = 3;
  grape::realtime::MPSCQueue<int> queue(CAPACITY);

  REQUIRE(queue.tryPush(1));
  REQUIRE(queue.tryPush(2));
  REQUIRE(queue.tryPush(3));

  REQUIRE(queue.count() == 3U);

  REQUIRE(queue.tryPop() == 1);
  REQUIRE(queue.tryPop() == 2);
  REQUIRE(queue.tryPop() == 3);

  REQUIRE(queue.count() == 0U);
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
    producers.emplace_back([&queue, i] -> void {
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

//-------------------------------------------------------------------------------------------------
TEST_CASE("A failed pop keeps the item available for retry", "[mpsc_queue]") {
  static constexpr auto EXPECTED = 42;
  grape::realtime::MPSCQueue<ThrowOnMove> queue(1U);
  REQUIRE(queue.tryPush(ThrowOnMove{ EXPECTED }));

  ThrowOnMove::throw_on_move = true;
  REQUIRE_THROWS_AS(queue.tryPop(), std::runtime_error);
  REQUIRE(queue.count() == 1U);

  const auto item = queue.tryPop();
  REQUIRE(item.has_value());
  if (item.has_value()) {
    REQUIRE(item->value == EXPECTED);
  }
  REQUIRE(queue.count() == 0U);
}

}  // namespace
