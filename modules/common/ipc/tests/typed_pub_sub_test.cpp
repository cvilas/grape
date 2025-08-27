//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <random>
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

  // define data callback
  std::condition_variable recv_cond;
  std::mutex recv_mut;
  auto received_data = TestDataType{};
  auto pub_id = grape::ipc::EntityId{};
  const auto data_cb = [&recv_cond, &recv_mut, &received_data, &pub_id](
                           const TestDataType& data, const grape::ipc::SampleInfo& info) -> void {
    const auto lk = std::scoped_lock{ recv_mut };
    received_data = data;
    pub_id = info.publisher;
    recv_cond.notify_all();
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
  publisher.publish(test_data);

  // wait for subscriber to receive it
  {
    constexpr auto RECV_WAIT_TIME = std::chrono::milliseconds(1000);
    auto lk = std::unique_lock(recv_mut);
    recv_cond.wait_for(lk, RECV_WAIT_TIME, [&pub_id] -> bool { return pub_id.id != 0U; });
  }

  // verify
  REQUIRE(received_data.id == test_data.id);
  REQUIRE(received_data.message == test_data.message);
}
