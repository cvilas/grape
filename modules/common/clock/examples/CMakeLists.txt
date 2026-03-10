# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

define_module_example(NAME example SOURCES example.cpp)

define_module_example(
  NAME bench
  SOURCES bench.cpp
  PRIVATE_LINK_LIBS benchmark::benchmark)
