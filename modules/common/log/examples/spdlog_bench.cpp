//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <print>

#include "grape/log/log.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

//-------------------------------------------------------------------------------------------------
auto benchmarkSpdlog(int num_iterations) -> std::chrono::milliseconds {
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto logger = std::make_shared<spdlog::logger>("benchmark_spdlogger", console_sink);

  auto start_time = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < num_iterations; ++i) {
    SPDLOG_LOGGER_INFO(logger, "Log message {}", i);
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
}

//-------------------------------------------------------------------------------------------------
auto benchmarkGrapeLog(int num_iterations) -> std::chrono::milliseconds {
  auto start_time = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < num_iterations; ++i) {
    grape::log::log(grape::log::Severity::Info, std::format("Log message {}", i));
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
}

//=================================================================================================
// Benchmark our logger against spdlog (https://github.com/gabime/spdlog)
auto main() -> int {
  constexpr int NUM_ITERATIONS = 100000;  // Number of log messages to output

  const auto spdlog_ms = benchmarkSpdlog(NUM_ITERATIONS);
  const auto grapelog_ms = benchmarkGrapeLog(NUM_ITERATIONS);

  std::println("spdlog - Elapsed time: {} ms", spdlog_ms.count());
  std::println("grape::log - Elapsed time: {} ms", grapelog_ms.count());

  return EXIT_SUCCESS;
}