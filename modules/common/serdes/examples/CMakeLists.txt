# =================================================================================================
# Copyright (C) 2024 GRAPE Contributors
# =================================================================================================

define_module_example(NAME example SOURCES example.cpp)

define_module_example(NAME variant_example SOURCES variant_example.cpp)

define_module_example(NAME advanced_example SOURCES advanced_example.cpp advanced_example.h)

define_module_example(
  NAME bench
  SOURCES bench.cpp advanced_example.h
  PRIVATE_LINK_LIBS benchmark::benchmark)
