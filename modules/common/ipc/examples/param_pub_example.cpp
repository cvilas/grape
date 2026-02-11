//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <cstddef>
#include <print>
#include <thread>

#include "grape/exception.h"
#include "grape/ipc/publisher.h"
#include "grape/ipc/session.h"
#include "topic_example.h"

//=================================================================================================
// A 'parameter publisher' that publishes constant data whenever a new subscriber is matched
template <grape::ipc::TopicAttributes Topic>
class ParameterPublisher {
public:
  ParameterPublisher(const Topic& topic_attr, typename Topic::DataType parameters)
    : parameters_(std::move(parameters))
    , publisher_(topic_attr, [this](const auto& match) { onMatch(match); }) {
  }

private:
  void onMatch(const grape::ipc::Match& match) {
    if (match.status == grape::ipc::Match::Status::Matched) {
      if (const auto result = publisher_.publish(parameters_); not result) {
        std::println(stderr, "Failed to publish parameters: {}", toString(result.error()));
      }
    }
  }

  typename Topic::DataType parameters_;
  grape::ipc::Publisher<Topic> publisher_;
};

//=================================================================================================
// Demonstrates a const parameter IPC publisher. See sub_example.cpp for matching subscriber.
auto main() -> int {
  try {
    grape::ipc::init(grape::ipc::Config{});

    const auto params = std::string("key=value");
    const auto topic = grape::ipc::ex::ExampleTopicAttributes{};
    auto server = ParameterPublisher(topic, params);

    constexpr auto SLEEP_TIME = std::chrono::milliseconds(500);
    while (grape::ipc::ok()) {
      std::this_thread::sleep_for(SLEEP_TIME);
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
