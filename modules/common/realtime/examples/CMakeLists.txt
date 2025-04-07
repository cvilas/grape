# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

define_module_example(NAME mpscq_example SOURCES mpscq_example.cpp)

define_module_example(
  NAME mpscq_bench
  SOURCES mpscq_bench.cpp
  PRIVATE_LINK_LIBS benchmark::benchmark
  PUBLIC_LINK_LIBS "")

define_module_example(NAME fifo_example SOURCES fifo_example.cpp)

define_module_example(
  NAME fifo_bench
  SOURCES fifo_bench.cpp
  PRIVATE_LINK_LIBS benchmark::benchmark
  PUBLIC_LINK_LIBS "")

define_module_example(NAME string_example SOURCES string_example.cpp)

define_module_example(NAME schedule_example SOURCES schedule_example.cpp)

define_module_example(NAME thread_example SOURCES thread_example.cpp)
