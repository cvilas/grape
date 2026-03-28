# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

define_module_example(NAME mpscq_example SOURCES mpscq_example.cpp)
define_module_example(NAME spmcq_example SOURCES spmcq_example.cpp)

define_module_example(
  NAME mpscq_bench
  SOURCES mpscq_bench.cpp
  PRIVATE_LINK_LIBS benchmark::benchmark
  PUBLIC_LINK_LIBS "")

define_module_example(
  NAME spmcq_bench
  SOURCES spmcq_bench.cpp
  PRIVATE_LINK_LIBS benchmark::benchmark
  PUBLIC_LINK_LIBS "")

define_module_example(NAME schedule_example SOURCES schedule_example.cpp)

define_module_example(NAME thread_example SOURCES thread_example.cpp)
