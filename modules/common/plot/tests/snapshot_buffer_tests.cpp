//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "grape/plot/snapshot_buffer.h"

namespace {

using Catch::Approx;

using SnapshotBuffer = grape::plot::SnapshotBuffer;
using Sample = grape::plot::Sample;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

//-------------------------------------------------------------------------------------------------
TEST_CASE("Capacity guard rejects invalid input", "[snapshot_buffer]") {
  REQUIRE_THROWS_AS(SnapshotBuffer(0), std::invalid_argument);
  REQUIRE_THROWS_AS(SnapshotBuffer(1), std::invalid_argument);
  REQUIRE_NOTHROW(SnapshotBuffer(2));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("pushBack builds buffer in chronological order", "[snapshot_buffer]") {
  SECTION("filling phase: size grows, capacity unchanged") {
    SnapshotBuffer buf(3);
    REQUIRE(buf.empty());
    REQUIRE(buf.capacity() == 3);

    buf.pushBack({ .x = 1, .y = 10 });
    REQUIRE_FALSE(buf.empty());
    REQUIRE(buf.size() == 1);

    buf.pushBack({ .x = 2, .y = 20 });
    REQUIRE(buf.size() == 2);

    buf.pushBack({ .x = 3, .y = 30 });
    REQUIRE(buf.size() == 3);  // full

    REQUIRE(buf.at(0).x == Approx(1.));
    REQUIRE(buf.at(1).x == Approx(2.));
    REQUIRE(buf.at(2).x == Approx(3.));
  }

  SECTION("overwrite phase: oldest element evicted, size stays at capacity") {
    SnapshotBuffer buf(3);
    buf.pushBack({ .x = 1, .y = 10 });
    buf.pushBack({ .x = 2, .y = 20 });
    buf.pushBack({ .x = 3, .y = 30 });

    buf.pushBack({ .x = 4, .y = 40 });  // evicts {1}
    REQUIRE(buf.size() == 3);
    REQUIRE(buf.at(0).x == Approx(2.));
    REQUIRE(buf.at(1).x == Approx(3.));
    REQUIRE(buf.at(2).x == Approx(4.));

    buf.pushBack({ .x = 5, .y = 50 });  // evicts {2}
    REQUIRE(buf.at(0).x == Approx(3.));
    REQUIRE(buf.at(1).x == Approx(4.));
    REQUIRE(buf.at(2).x == Approx(5.));

    buf.pushBack({ .x = 6, .y = 60 });  // evicts {3}; head wraps to 0
    REQUIRE(buf.at(0).x == Approx(4.));
    REQUIRE(buf.at(1).x == Approx(5.));
    REQUIRE(buf.at(2).x == Approx(6.));
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("view() returns chronological spans", "[snapshot_buffer]") {
  SECTION("empty buffer returns empty spans") {
    const SnapshotBuffer buf(3);
    const auto [s1, s2] = buf.view();
    REQUIRE(s1.empty());
    REQUIRE(s2.empty());
  }

  SECTION("partial fill returns one contiguous span") {
    SnapshotBuffer buf(3);
    buf.pushBack({ .x = 1, .y = 10 });
    buf.pushBack({ .x = 2, .y = 20 });
    const auto [s1, s2] = buf.view();
    REQUIRE(s1.size() == 2);
    REQUIRE(s2.empty());
    REQUIRE(s1.at(0).x == Approx(1.));
    REQUIRE(s1.at(1).x == Approx(2.));
  }

  SECTION("exactly full, unrotated returns one contiguous span") {
    SnapshotBuffer buf(3);
    buf.pushBack({ .x = 1, .y = 10 });
    buf.pushBack({ .x = 2, .y = 20 });
    buf.pushBack({ .x = 3, .y = 30 });
    const auto [s1, s2] = buf.view();
    REQUIRE(s1.size() == 3);
    REQUIRE(s2.empty());
    REQUIRE(s1.at(0).x == Approx(1.));
    REQUIRE(s1.at(1).x == Approx(2.));
    REQUIRE(s1.at(2).x == Approx(3.));
  }

  SECTION("wrapped buffer splits across two spans") {
    SnapshotBuffer buf(3);
    buf.pushBack({ .x = 1, .y = 10 });
    buf.pushBack({ .x = 2, .y = 20 });
    buf.pushBack({ .x = 3, .y = 30 });
    buf.pushBack({ .x = 4, .y = 40 });  // head=1; storage=[{4},{2},{3}]
    const auto [s1, s2] = buf.view();
    // s1 covers physical [1,2] → {2},{3}; s2 covers physical [0] → {4}
    REQUIRE(s1.size() == 2);
    REQUIRE(s2.size() == 1);
    REQUIRE(s1.at(0).x == Approx(2.));
    REQUIRE(s1.at(1).x == Approx(3.));
    REQUIRE(s2.at(0).x == Approx(4.));
  }

  SECTION("fully rotated buffer returns one contiguous span again") {
    SnapshotBuffer buf(3);
    for (std::size_t idx = 1; idx <= 6; ++idx) {
      const auto val = static_cast<double>(idx);
      buf.pushBack({ .x = val, .y = val * 10.0 });
    }
    // After 6 pushes into cap-3: head wraps back to 0; storage=[{4},{5},{6}]
    const auto [s1, s2] = buf.view();
    REQUIRE(s1.size() == 3);
    REQUIRE(s2.empty());
    REQUIRE(s1.at(0).x == Approx(4.));
    REQUIRE(s1.at(1).x == Approx(5.));
    REQUIRE(s1.at(2).x == Approx(6.));
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("front() and back() track oldest and newest elements", "[snapshot_buffer]") {
  SnapshotBuffer buf(3);

  buf.pushBack({ .x = 1, .y = 10 });
  REQUIRE(buf.front().x == Approx(1.));
  REQUIRE(buf.back().x == Approx(1.));

  buf.pushBack({ .x = 2, .y = 20 });
  REQUIRE(buf.front().x == Approx(1.));
  REQUIRE(buf.back().x == Approx(2.));

  buf.pushBack({ .x = 3, .y = 30 });
  REQUIRE(buf.front().x == Approx(1.));
  REQUIRE(buf.back().x == Approx(3.));

  buf.pushBack({ .x = 4, .y = 40 });  // evicts {1}
  REQUIRE(buf.front().x == Approx(2.));
  REQUIRE(buf.back().x == Approx(4.));

  buf.pushBack({ .x = 5, .y = 50 });  // evicts {2}
  REQUIRE(buf.front().x == Approx(3.));
  REQUIRE(buf.back().x == Approx(5.));

  buf.pushBack({ .x = 6, .y = 60 });  // evicts {3}; head wraps to 0
  REQUIRE(buf.front().x == Approx(4.));
  REQUIRE(buf.back().x == Approx(6.));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("clear() resets state and allows refill from scratch", "[snapshot_buffer]") {
  SnapshotBuffer buf(3);
  buf.pushBack({ .x = 1, .y = 10 });
  buf.pushBack({ .x = 2, .y = 20 });

  buf.clear();
  REQUIRE(buf.empty());

  buf.pushBack({ .x = 3, .y = 30 });
  REQUIRE(buf.size() == 1);
  REQUIRE(buf.front().x == Approx(3.));
  REQUIRE(buf.back().x == Approx(3.));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("at() provides bounds-checked element access", "[snapshot_buffer]") {
  SnapshotBuffer buf(3);
  buf.pushBack({ .x = 1, .y = 10 });

  REQUIRE_NOTHROW(buf.at(0));
  REQUIRE(buf.at(0).x == Approx(1.));
  REQUIRE_THROWS_AS(buf.at(1), std::out_of_range);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace
