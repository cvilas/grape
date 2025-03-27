//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <random>
#include <thread>
#include <vector>

#include "catch2/catch_test_macros.hpp"
#include "grape/ipc/session.h"

namespace {

// NOLINTBEGIN(cert-err58-cpp)

//=================================================================================================
TEST_CASE("A process can have only one session", "[ipc]") {
  auto session = grape::ipc::Session(grape::ipc::Session::Config{});
  REQUIRE_THROWS(std::make_unique<grape::ipc::Session>(grape::ipc::Session::Config{}));
}

//=================================================================================================
TEST_CASE("Basic pub-sub on large message works", "[ipc]") {
  auto session = grape::ipc::Session(grape::ipc::Session::Config{});
  const auto topic = grape::ipc::Topic{ .name = "pub_sub_test" };

  // Create a large payload (eg: 1080p RGB image)
  constexpr auto PAYLOAD_SIZE = 1920U * 1080U * 3;
  auto payload = std::vector<std::byte>(PAYLOAD_SIZE);

  // Set random pattern in the payload
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(CHAR_MIN, CHAR_MAX);
  std::ranges::generate(payload, [&gen, &dis]() { return static_cast<std::byte>(dis(gen)); });

  // define subscriber callback
  std::condition_variable recv_cond;
  std::mutex recv_mut;
  auto received_msg = std::vector<std::byte>{};
  const auto recv_callback = [&recv_mut, &recv_cond,
                              &received_msg](const grape::ipc::Sample& sample) {
    const std::lock_guard<std::mutex> lk(recv_mut);
    received_msg = std::vector<std::byte>(sample.data.begin(), sample.data.end());
    recv_cond.notify_all();
  };

  // create pub/sub
  auto publisher = session.createPublisher(topic);
  auto subscriber = session.createSubscriber(topic.name, recv_callback);

  // Wait for pub/sub registration
  constexpr auto RETRY_COUNT = 10U;
  auto count_down = RETRY_COUNT;
  while ((subscriber.getPublisherCount() == 0) && (count_down > 0)) {
    constexpr auto REG_WAIT_TIME = std::chrono::milliseconds(200);
    std::this_thread::sleep_for(REG_WAIT_TIME);
    count_down--;
  }
  REQUIRE(subscriber.getPublisherCount() == 1);

  // publish payload
  publisher.publish({ payload.data(), payload.size() });

  // wait a reasonable time for subscriber to receive message
  constexpr auto RECV_WAIT_TIME = std::chrono::milliseconds(3000);
  std::unique_lock lk(recv_mut);
  recv_cond.wait_for(lk, RECV_WAIT_TIME, [&received_msg] { return not received_msg.empty(); });

  // verify message
  REQUIRE(received_msg.size() == PAYLOAD_SIZE);
  REQUIRE(received_msg == payload);
}

// NOLINTEND(cert-err58-cpp)

}  // namespace
