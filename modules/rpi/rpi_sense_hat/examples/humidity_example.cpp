//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <print>
#include <thread>

#include "grape/exception.h"
#include "grape/rpi/sense_hat/humidity_sensor.h"

//=================================================================================================
auto main() -> int {
  try {
    auto sensor = grape::rpi::sense_hat::HumiditySensor({});

    static constexpr auto DT = std::chrono::seconds(1);
    auto next_ts = grape::WallClock::now();
    while (true) {
      const auto maybe_sample = sensor.read();
      if (maybe_sample.has_value()) {
        const auto& sample = maybe_sample.value();
        std::println("[{}] {:.2f} % RH, {:.2f} °C",     //
                     sample.timestamp,                  //
                     sample.relative_humidity_percent,  //
                     sample.temperature_celsius);
      } else {
        std::println("{}", maybe_sample.error().message());
      }
      next_ts += DT;
      std::this_thread::sleep_until(next_ts);
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
