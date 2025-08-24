//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <algorithm>
#include <condition_variable>
#include <random>
#include <thread>
#include <vector>

#include "catch2/catch_test_macros.hpp"
#include "grape/ipc/raw_publisher.h"
#include "grape/ipc/raw_subscriber.h"
#include "grape/ipc/session.h"

namespace {

// NOLINTBEGIN(cert-err58-cpp)

//=================================================================================================
TEST_CASE("Basic pub-sub on large message works in host-only scope", "[ipc]") {
  grape::ipc::init(grape::ipc::Config{});
  const auto* const topic = "pub_sub_test";

  // Create a large payload (eg: 1080p RGB image)
  constexpr auto PAYLOAD_SIZE = 1920U * 1080U * 3;
  auto payload = std::vector<std::byte>(PAYLOAD_SIZE);

  // Set random pattern in the payload
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(CHAR_MIN, CHAR_MAX);
  std::ranges::generate(payload,
                        [&gen, &dis]() -> std::byte { return static_cast<std::byte>(dis(gen)); });

  // define subscriber callback
  std::condition_variable recv_cond;
  std::mutex recv_mut;
  auto received_msg = std::vector<std::byte>{};
  auto pub_id = 0UL;
  const auto recv_callback = [&recv_mut, &recv_cond, &received_msg,
                              &pub_id](const grape::ipc::Sample& sample) -> void {
    const auto lk = std::lock_guard(recv_mut);
    received_msg = std::vector<std::byte>(sample.data.begin(), sample.data.end());
    pub_id = sample.info.publisher.id;
    recv_cond.notify_all();
  };

  // create pub/sub
  auto matched_sub_id = 0UL;
  const auto pub_match_cb = [&matched_sub_id](const grape::ipc::Match& match) -> void {
    matched_sub_id = match.remote_entity.id;
  };
  auto publisher = grape::ipc::RawPublisher(topic, pub_match_cb);

  auto matched_pub_id = 0UL;
  const auto sub_match_cb = [&matched_pub_id](const grape::ipc::Match& match) -> void {
    matched_pub_id = match.remote_entity.id;
  };
  auto subscriber = grape::ipc::RawSubscriber(topic, recv_callback, sub_match_cb);

  // Wait for pub/sub registration
  constexpr auto RETRY_COUNT = 10U;
  auto count_down = RETRY_COUNT;
  while ((subscriber.publisherCount() == 0) && (count_down > 0)) {
    constexpr auto REG_WAIT_TIME = std::chrono::milliseconds(200);
    std::this_thread::sleep_for(REG_WAIT_TIME);
    count_down--;
  }
  REQUIRE(subscriber.publisherCount() == 1);
  REQUIRE(matched_sub_id != 0UL);
  REQUIRE(matched_pub_id != 0UL);
  REQUIRE(matched_sub_id == subscriber.id());
  REQUIRE(matched_pub_id == publisher.id());

  // publish payload
  publisher.publish({ payload.data(), payload.size() });

  // wait a reasonable time for subscriber to receive message
  {
    constexpr auto RECV_WAIT_TIME = std::chrono::milliseconds(1000);
    auto lk = std::unique_lock(recv_mut);
    recv_cond.wait_for(lk, RECV_WAIT_TIME,
                       [&received_msg] -> bool { return not received_msg.empty(); });
  }

  // verify message
  REQUIRE(received_msg.size() == PAYLOAD_SIZE);
  REQUIRE(received_msg == payload);
  REQUIRE(pub_id == publisher.id());
}

// NOLINTEND(cert-err58-cpp)

}  // namespace
