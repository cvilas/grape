//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <algorithm>
#include <random>
#include <semaphore>
#include <thread>
#include <vector>

#include "catch2/catch_test_macros.hpp"
#include "grape/ipc/publisher.h"
#include "grape/ipc/session.h"
#include "grape/ipc/subscriber.h"

namespace {
struct TestDataType {
  std::uint64_t id{};
  std::string message;
};

struct TestTopicAttributes {
  using DataType = TestDataType;
  static constexpr auto SERDES_BUFFER_SIZE = 1024U;
  static auto topicName() -> std::string {
    return "typed_pub_sub_test";
  }
};

template <grape::serdes::WritableStream Stream>
auto serialise(grape::serdes::Serialiser<Stream>& ser, const TestDataType& st) -> bool {
  return ser.pack(st.id) and ser.pack(st.message);
}

template <grape::serdes::ReadableStream Stream>
[[nodiscard]] auto deserialise(grape::serdes::Deserialiser<Stream>& des, TestDataType& st) -> bool {
  return des.unpack(st.id) and des.unpack(st.message);
}

}  // namespace

//=================================================================================================
TEST_CASE("Basic functionality of pub-sub templated on topic attributes", "[ipc]") {
  grape::ipc::init(grape::ipc::Config{});

  std::binary_semaphore is_data_received{ 0 };
  auto received_data = TestDataType{};
  auto pub_id = grape::ipc::EntityId{};

  const auto data_cb = [&is_data_received, &received_data,
                        &pub_id](const std::expected<TestDataType, grape::ipc::Error>& data,
                                 const grape::ipc::SampleInfo& info) {
    if (not data) {
      return;
    }
    received_data = data.value();
    pub_id = info.publisher;
    is_data_received.release();
  };

  // create pub/sub
  auto publisher = grape::ipc::Publisher(TestTopicAttributes{});
  auto subscriber = grape::ipc::Subscriber(TestTopicAttributes{}, data_cb);

  // wait for match
  constexpr auto RETRY_COUNT = 10U;
  auto count_down = RETRY_COUNT;
  while ((subscriber.publisherCount() == 0U) && (count_down > 0)) {
    constexpr auto REG_WAIT_TIME = std::chrono::milliseconds(200);
    std::this_thread::sleep_for(REG_WAIT_TIME);
    count_down--;
  }
  REQUIRE(subscriber.publisherCount() == 1U);

  // create test data
  const auto test_data = TestDataType{ .id = 42, .message = "Custom struct message" };

  // publish it
  REQUIRE(publisher.publish(test_data).has_value());

  // wait for subscriber to receive it with timeout
  constexpr auto RECV_WAIT_TIME = std::chrono::milliseconds(1000);
  const auto success = is_data_received.try_acquire_for(RECV_WAIT_TIME);
  REQUIRE(success);

  // verify
  REQUIRE(received_data.id == test_data.id);
  REQUIRE(received_data.message == test_data.message);
}
