//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <climits>
#include <csignal>
#include <print>
#include <system_error>
#include <thread>

#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "grape/exception.h"
#include "grape/realtime/shared_memory.h"
#include "grape/wall_clock.h"

namespace {

//=================================================================================================
// Data structure whose instance is shared between producer and consumer
struct Signal {
  alignas(std::int64_t) std::atomic<std::int64_t> nanos{ 0 };
  [[nodiscard]] auto get() const -> std::int64_t;
  void post();
  [[nodiscard]] auto wait(std::int64_t expected, std::chrono::milliseconds timeout) const -> bool;
};

//-------------------------------------------------------------------------------------------------
auto Signal::get() const -> std::int64_t {
  return nanos.load(std::memory_order_acquire);
}

//-------------------------------------------------------------------------------------------------
void Signal::post() {
  nanos.store(grape::WallClock::toNanos(grape::WallClock::now()), std::memory_order_release);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  const auto result = syscall(SYS_futex, &nanos, FUTEX_WAKE, INT_MAX, nullptr, nullptr, 0);
  if (result == -1) {
    const auto err = std::error_code(errno, std::system_category());
    throw std::runtime_error("(futex_wake) " + err.message());
  }
}

//-------------------------------------------------------------------------------------------------
auto Signal::wait(std::int64_t expected, std::chrono::milliseconds timeout) const -> bool {
  auto sec = std::chrono::duration_cast<std::chrono::seconds>(timeout);
  auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout - sec);
  const auto ts = timespec{ .tv_sec = sec.count(), .tv_nsec = nsec.count() };

  // Futex operates on 32-bit values and we use it only to detect change
  const auto ex = static_cast<std::uint32_t>(expected);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  const auto result = syscall(SYS_futex, &nanos, FUTEX_WAIT, ex, &ts, nullptr, 0);
  if (result == -1) {
    const auto err = std::error_code(errno, std::system_category());
    if (err.value() == ETIMEDOUT) {
      return false;  // timeout
    }
    if (err.value() == EINTR) {
      return false;  // interrupted
    }
    if (err.value() == EAGAIN) {
      return true;  // value not the expected one
    }
    throw std::runtime_error("(futex_wait) " + err.message());
  }
  return true;
}

using Shm = grape::realtime::SharedMemory;

constexpr auto SHM_NAME = "/grape_shm_futex_example";
constexpr auto SHM_SIZE = sizeof(Signal);

//-------------------------------------------------------------------------------------------------
auto initShm(Shm::Access access) -> std::optional<Shm> {
  // Try to create shared memory. If that fails, try to open. If that fails, give up
  std::print("Creating shared memory..");
  auto maybe_shm = Shm::create(SHM_NAME, SHM_SIZE, access);
  const auto shm_created = maybe_shm.has_value();
  if (not shm_created) {
    std::println("..failed: {}", maybe_shm.error().message());
    std::print("Opening shared memory..");
    maybe_shm = Shm::open(SHM_NAME, access);
    if (not maybe_shm) {
      std::println("..failed: {}", maybe_shm.error().message());
      return std::nullopt;
    }
  }
  std::println("OK");
  return std::move(maybe_shm.value());
}

//-------------------------------------------------------------------------------------------------
std::atomic_bool s_exit = false;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
void onSignal(int /*signum*/) {
  s_exit = true;
}

//-------------------------------------------------------------------------------------------------
// Producer: Generates data and signals consumer
void produce() {
  std::println("Producer: Start");
  auto maybe_shm = initShm(Shm::Access::ReadWrite);
  if (not maybe_shm) {
    return;
  }

  auto& shm = maybe_shm.value();

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* signal = reinterpret_cast<Signal*>(shm.data().data());

  static constexpr auto PRODUCE_INTERVAL = std::chrono::milliseconds(500);
  while (not s_exit) {
    signal->post();
    std::println("Producer: nanos={}", signal->get());
    std::this_thread::sleep_for(PRODUCE_INTERVAL);
  }

  shm.close();
  std::ignore = Shm::remove(SHM_NAME);
  std::println("Producer: Exit");
}

//-------------------------------------------------------------------------------------------------
// Consumer: Waits for signal from producer and processes data
void consume() {
  std::println("Consumer: Start");
  auto maybe_shm = initShm(Shm::Access::ReadOnly);
  if (not maybe_shm) {
    return;
  }

  auto& shm = maybe_shm.value();

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  const auto* signal = reinterpret_cast<const Signal*>(shm.data().data());

  static constexpr auto WAIT_TIMEOUT = std::chrono::milliseconds(2000);
  auto last_seq_num = signal->get();
  while (not s_exit) {
    if (signal->wait(last_seq_num, WAIT_TIMEOUT)) {
      last_seq_num = signal->get();
      const auto latency = grape::WallClock::toNanos(grape::WallClock::now()) - last_seq_num;
      std::println("Consumer: Received nanos={}, latency={} ns", last_seq_num, latency);
    }
  }

  shm.close();
  std::println("Consumer: Exit");
}

}  // namespace

//=================================================================================================
// Example program demonstrates producer-consumer communication across shared memory using futex.
//
// - Creates a shared memory region as cross-process communication channel
// - Producer generates timestamped data and signals availability
// - Consumer waits for data with timeout and processes it
//
// Run this program in two terminals:
// Terminal 1 (producer): grape_realtime_shm_futex_example produce
// Terminal 2 (consumer): grape_realtime_shm_futex_example consume
//
auto main(int argc, char* argv[]) -> int {
  auto args = std::span(argv, static_cast<std::size_t>(argc));
  (void)signal(SIGINT, onSignal);
  (void)signal(SIGTERM, onSignal);
  try {
    if (argc != 2) {
      std::println("Usage: {} <produce|consume>", args.at(0));
      return EXIT_FAILURE;
    }
    const std::string mode(args.at(1));
    std::println("Press Ctrl-C to exit");
    if (mode == "produce") {
      produce();
    } else if (mode == "consume") {
      consume();
    } else {
      std::println("Error: Invalid mode '{}'. Use 'produce' or 'consume'.", mode);
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
