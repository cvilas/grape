//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <csignal>
#include <cstring>
#include <print>

#include "grape/fifo_buffer.h"
#include "grape/realtime/schedule.h"
#include "grape/realtime/thread.h"
#include "grape/rpi/sense_hat/inertial_sensor.h"

namespace {

std::atomic_flag s_exit = false;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

void onSignal(int /*signum*/) {
  s_exit.test_and_set();
  s_exit.notify_one();
}

}  // namespace

//=================================================================================================
auto main() -> int {
  std::ignore = signal(SIGINT, onSignal);
  std::ignore = signal(SIGTERM, onSignal);

  try {
    const auto is_mem_locked = grape::realtime::lockMemory();
    if (not is_mem_locked) {
      std::println("Unable to lock memory: {}", is_mem_locked.error().message());
    }

    static constexpr auto CPUS_RT = { 2U };
    static constexpr auto CPUS_NON_RT = { 0U, 1U };

    // Pin main thread to housekeeping CPUs
    const auto is_main_cpu_set = grape::realtime::setCpuAffinity(CPUS_NON_RT);
    if (not is_main_cpu_set) {
      std::println("Unable to set CPU affinity (main): {}", is_main_cpu_set.error().message());
    }

    // A lock-free buffer to pass data from IMU reading thread to main thread
    static constexpr auto QUEUE_CAPACITY = 64U;
    auto sample_buffer = grape::FIFOBuffer(grape::FIFOBuffer::Config{
        .frame_length = sizeof(grape::rpi::sense_hat::ImuSample),
        .num_frames = QUEUE_CAPACITY,
    });

    // Create IMU driver instance
    using IMU = grape::rpi::sense_hat::InertialSensor;
    auto imu = IMU(IMU::Config{
        .i2c_bus = "/dev/i2c-1",
    });

    // Configure IMU reading thread
    auto task_config = grape::realtime::Thread::Config();
    task_config.name = "imu_rt";
    static constexpr auto PROCESS_INTERVAL = std::chrono::microseconds(2000);
    task_config.interval = PROCESS_INTERVAL;

    task_config.setup = []() -> bool {
      const auto is_cpu_set = grape::realtime::setCpuAffinity(CPUS_RT);
      if (not is_cpu_set) {
        std::println("Unable to set CPU affinity (thread): {}", is_cpu_set.error().message());
      }

      // Set real-time scheduling
      static constexpr auto RT_PRIORITY = 80;
      const auto is_scheduled = grape::realtime::setSchedule(
          { .policy = grape::realtime::Schedule::Policy::Realtime, .priority = RT_PRIORITY });
      if (not is_scheduled) {
        std::println("Unable to set RT schedule (thread): {}", is_scheduled.error().message());
      }
      return true;
    };

    task_config.process = [&imu, &sample_buffer]() -> bool {
      // read the IMU
      auto maybe_sample = imu.read();
      if (not maybe_sample) {
        std::println("IMU read failed: {}", maybe_sample.error().message());
        return false;
      }

      // Push raw IMU data to buffer read by main thread
      const auto writer = [&maybe_sample](std::span<std::byte> frame) {
        std::memcpy(frame.data(), &*maybe_sample, frame.size_bytes());
      };
      if (not sample_buffer.visitToWrite(writer)) {
        std::println("FIFO full");
      }

      return true;
    };

    // Launch IMU reading thread
    auto task = grape::realtime::Thread(std::move(task_config));
    task.start();

    auto sample = grape::rpi::sense_hat::ImuSample{};
    const auto reader = [&sample](std::span<const std::byte> frame) {
      std::memcpy(&sample, frame.data(), frame.size_bytes());
    };

    // Main thread just logs data
    std::println("Press ctrl-c to exit\n");
    static constexpr auto LOG_PERIOD = std::chrono::seconds(1);
    auto next_log_time = grape::WallClock::now();
    while (not s_exit.test()) {
      if (sample_buffer.visitToRead(reader)) {
        if (sample.timestamp < next_log_time) {
          continue;
        }
        std::println("[{}] gyro=({:>6d},{:>6d},{:>6d}) "
                     "accel=({:>6d},{:>6d},{:>6d}) "
                     "mag=({:>6d},{:>6d},{:>6d}){}",
                     sample.timestamp, sample.gyro.x, sample.gyro.y, sample.gyro.z, sample.accel.x,
                     sample.accel.y, sample.accel.z, sample.mag.x, sample.mag.y, sample.mag.z,
                     sample.mag_updated ? " [M]" : "");
        next_log_time += LOG_PERIOD;
      } else {
        static constexpr auto YIELD_TIME = std::chrono::milliseconds(1);
        std::this_thread::sleep_for(YIELD_TIME);
      }
    }

    task.stop();
    return EXIT_SUCCESS;

  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
