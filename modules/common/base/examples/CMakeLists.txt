# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

define_module_example(NAME exception_example SOURCES exception_example.cpp)
define_module_example(NAME fifo_example SOURCES fifo_example.cpp)
define_module_example(
  NAME fifo_bench
  SOURCES fifo_bench.cpp
  PRIVATE_LINK_LIBS benchmark::benchmark)
define_module_example(NAME string_example SOURCES string_example.cpp)
define_module_example(NAME error_example SOURCES error_example.cpp)
define_module_example(NAME wall_timer_example SOURCES wall_timer_example.cpp)
