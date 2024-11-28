//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#define SPDLOG_USE_STD_FORMAT

#include <fstream>

#include <benchmark/benchmark.h>

#include "grape/log/logger.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

namespace {

//-------------------------------------------------------------------------------------------------
auto bmSpdlog(benchmark::State& state) {
  auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("benchmark_spdlog_out.txt", true);
  auto logger = std::make_shared<spdlog::logger>("benchmark_spdlogger", sink);
  auto i = 0uz;
  for (auto s : state) {
    (void)s;
    SPDLOG_LOGGER_INFO(logger, "Log number {:d}", i++);
  }
}

//-------------------------------------------------------------------------------------------------
auto bmGrapeLog(benchmark::State& state) {
  auto log_file = std::ofstream("benchmark_grapelog_out.txt");
  auto config = grape::log::Config();
  config.queue_capacity = static_cast<std::size_t>(state.max_iterations);
  config.flush_period = std::chrono::microseconds(100);
  config.sink = [&log_file](const grape::log::Record& r) {
    log_file << grape::log::defaultFormatter(r) << '\n';
  };
  config.logger_name = "benchmark_grapelog";

  auto logger = grape::log::Logger(std::move(config));
  auto i = 0uz;
  for (auto s : state) {
    (void)s;
    grape::log::Log(logger, grape::log::Severity::Info, "Log number {:d}", i++);
  }
}

constexpr auto MAX_ITERATIONS = 1000000U;

BENCHMARK(bmSpdlog)->Iterations(MAX_ITERATIONS);
BENCHMARK(bmGrapeLog)->Iterations(MAX_ITERATIONS);
}  // namespace
BENCHMARK_MAIN();
