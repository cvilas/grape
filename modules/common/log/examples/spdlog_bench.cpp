//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <fstream>

#include "grape/log/logger.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

//-------------------------------------------------------------------------------------------------
auto benchmarkSpdlog(std::size_t num_iterations) -> std::chrono::microseconds {
  auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("benchmark_spdlog_out.txt", true);
  auto logger = std::make_shared<spdlog::logger>("benchmark_spdlogger", sink);

  const auto start_time = std::chrono::high_resolution_clock::now();
  for (std::size_t i = 0; i < num_iterations; ++i) {
    SPDLOG_LOGGER_INFO(logger, "Log message {:d}", i);
  }
  const auto end_time = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
}

//-------------------------------------------------------------------------------------------------
auto benchmarkGrapeLog(std::size_t num_iterations) -> std::chrono::microseconds {
  auto log_file = std::ofstream("benchmark_grapelog_out.txt");
  auto config = grape::log::Config();
  config.queue_capacity = num_iterations;
  config.flush_period = std::chrono::microseconds(100);
  config.sink = [&log_file](const grape::log::Record& r) {
    log_file << grape::log::defaultFormatter(r) << '\n';
  };
  config.logger_name = "benchmark_grapelog";

  auto logger = std::make_unique<grape::log::Logger>(std::move(config));
  const auto start_time = std::chrono::high_resolution_clock::now();
  for (std::size_t i = 0; i < num_iterations; ++i) {
    logger->log(grape::log::Severity::Info, std::format("Log message {:d}", i));
  }
  const auto end_time = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
}

//=================================================================================================
// Benchmark our logger against spdlog (https://github.com/gabime/spdlog)
auto main() -> int {
  constexpr std::size_t NUM_ITERATIONS = 100000;  // Number of log messages to output

  const auto spdlog = benchmarkSpdlog(NUM_ITERATIONS);
  const auto log = benchmarkGrapeLog(NUM_ITERATIONS);

  std::println("Benchmark {} iterations:", NUM_ITERATIONS);
  std::println("  spdlog {} us", spdlog.count());
  std::println("  grape  {} us", log.count());

  return EXIT_SUCCESS;
}