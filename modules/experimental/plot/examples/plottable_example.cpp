//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <chrono>
#include <iostream>

#include "grape/plot/plottable.h"

using namespace std::chrono_literals;

//=================================================================================================
// Example plottable data structures
//=================================================================================================

using SteadyNs = std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds>;
using SteadyUs = std::chrono::time_point<std::chrono::steady_clock, std::chrono::microseconds>;

struct TemperatureSensor {
  SteadyNs timestamp;
  double temperature_celsius;
  int32_t sensor_id;
};

struct VehicleState {
  SteadyUs timestamp;
  double position_x;
  double position_y;
  float velocity;
  float heading;
  uint8_t gear;
};

//=================================================================================================
// Generic function that works with any Plottable type
//=================================================================================================

template <grape::Plottable T>
void print_data_info(const T& data) {
  std::cout << "Timestamp: " << data.timestamp.time_since_epoch().count() << " (size: " << sizeof(T)
            << " bytes)\n";
}

//=================================================================================================
// Main
//=================================================================================================

auto main() -> int {
  std::cout << "Plottable Concept Example\n";
  std::cout << "=========================\n\n";

  // Create and use a temperature sensor reading
  TemperatureSensor temp{ .timestamp = SteadyNs{ 1'000'000ns },
                          .temperature_celsius = 23.5,
                          .sensor_id = 42 };

  std::cout << "Temperature Sensor Data:\n";
  print_data_info(temp);
  std::cout << "  Temperature: " << temp.temperature_celsius << "°C\n";
  std::cout << "  Sensor ID: " << temp.sensor_id << "\n\n";

  // Create and use vehicle state data
  VehicleState vehicle{ .timestamp = SteadyUs{ 2'500us },
                        .position_x = 10.5,
                        .position_y = 20.3,
                        .velocity = 15.2f,
                        .heading = 1.57f,
                        .gear = 3 };

  std::cout << "Vehicle State Data:\n";
  print_data_info(vehicle);
  std::cout << "  Position: (" << vehicle.position_x << ", " << vehicle.position_y << ")\n";
  std::cout << "  Velocity: " << vehicle.velocity << " m/s\n";
  std::cout << "  Heading: " << vehicle.heading << " rad\n";
  std::cout << "  Gear: " << static_cast<int>(vehicle.gear) << "\n\n";

  // Compile-time validation
  static_assert(grape::Plottable<TemperatureSensor>, "TemperatureSensor should be Plottable");
  static_assert(grape::Plottable<VehicleState>, "VehicleState should be Plottable");

  std::cout << "All data types satisfy the Plottable concept!\n";

  return 0;
}
