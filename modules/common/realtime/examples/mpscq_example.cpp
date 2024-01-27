//=================================================================================================
// Copyright (C) 2018 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <csignal>
#include <print>
#include <thread>
#include <vector>

#include "grape/realtime/mpsc_queue.h"

namespace {

//-------------------------------------------------------------------------------------------------
std::atomic_bool s_exit = false;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

void onSignal(int /*signum*/) {
  s_exit = true;
}

}  // namespace

//=================================================================================================
auto main() -> int {
  try {
    // An example data structure
    struct ExampleObject {
      std::string name;
      std::uint64_t value{};
    };

    static constexpr std::size_t Q_LEN = 10;
    grape::realtime::MPSCQueue<ExampleObject> mpscq(Q_LEN);

    const auto producer = [&mpscq](const std::string& name) {
      static constexpr auto UPDATE_PERIOD = std::chrono::milliseconds(100);
      std::uint64_t value = 0;
      while (!s_exit) {
        ExampleObject obj{ .name = name, .value = value };
        const auto pushed = mpscq.tryPush(std::move(obj));
        std::this_thread::sleep_for(UPDATE_PERIOD);
        if (pushed) {
          std::println("produce - [{}] {}", name, value);
          ++value;
        } else {
          std::println("produce - [{}] {}", name, "busy");
        }
      }
    };

    const auto consumer = [&mpscq]() {
      static constexpr auto REST_PERIOD = std::chrono::seconds(1);
      while (!s_exit) {
        if (mpscq.count() > 0) {
          const auto obj_opt = mpscq.tryPop();
          if (obj_opt.has_value()) {
            const auto& obj = obj_opt.value();
            std::println("consume - {:d} [{:s}]", obj.value, obj.name);
          } else {
            std::println("consume - empty");
            std::this_thread::sleep_for(REST_PERIOD);
          }
        }
      }
    };

    (void)signal(SIGINT, onSignal);
    (void)signal(SIGTERM, onSignal);

    std::thread t1(producer, "P1");
    std::thread t2(producer, "P2");
    consumer();

    t1.join();
    t2.join();
  } catch (const std::exception& ex) {
    std::ignore = fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
