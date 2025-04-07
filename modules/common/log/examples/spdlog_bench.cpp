//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#define SPDLOG_USE_STD_FORMAT

#include <format>

#include <benchmark/benchmark.h>

#include "grape/log/sinks/file_sink.h"
#include "grape/log/syslog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

namespace {

//-------------------------------------------------------------------------------------------------
auto bmSpdlog(benchmark::State& state) {
  auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("benchmark_spdlog_out.txt", true);
  auto logger = std::make_shared<spdlog::logger>("benchmark_spdlogger", sink);
  auto i = 0uz;
  for (auto _ : state) {
    SPDLOG_LOGGER_INFO(logger, "Log number {:d}", i++);
  }
}

//-------------------------------------------------------------------------------------------------
auto bmGrapeLog(benchmark::State& state) {
  using LogSink = grape::log::FileSink<grape::log::DefaultFormatter>;
  auto config = grape::log::Config();
  config.queue_capacity = static_cast<std::size_t>(state.max_iterations);
  config.flush_period = std::chrono::microseconds(100);
  config.sink = std::make_shared<LogSink>("benchmark_grapelog_out.txt", false);
  config.logger_name = "benchmark_grapelog";

  auto logger = grape::log::Logger(std::move(config));
  auto i = 0uz;
  for (auto _ : state) {
    grape::log::Log(logger, grape::log::Severity::Info, "Log number {:d}", i++);
  }
}

//-------------------------------------------------------------------------------------------------
auto bmGrapeSystemLog(benchmark::State& state) {
  using LogSink = grape::log::FileSink<grape::log::DefaultFormatter>;
  auto config = grape::log::Config();
  config.queue_capacity = static_cast<std::size_t>(state.max_iterations);
  config.flush_period = std::chrono::microseconds(100);
  config.sink = std::make_shared<LogSink>("benchmark_grapesylog_out.txt", false);
  config.logger_name = "benchmark_grapesyslog";

  grape::syslog::init(std::move(config));
  auto i = 0uz;
  for (auto _ : state) {
    grape::syslog::Log(grape::log::Severity::Info, "Log number {:d}", i++);
  }
}

constexpr auto MAX_ITERATIONS = 1000000U;

BENCHMARK(bmSpdlog)->Iterations(MAX_ITERATIONS);
BENCHMARK(bmGrapeLog)->Iterations(MAX_ITERATIONS);
BENCHMARK(bmGrapeSystemLog)->Iterations(MAX_ITERATIONS);

}  // namespace
BENCHMARK_MAIN();
