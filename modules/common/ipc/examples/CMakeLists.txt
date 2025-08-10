# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

define_module_example(NAME raw_pub_example SOURCES raw_pub_example.cpp)

define_module_example(NAME raw_sub_example SOURCES raw_sub_example.cpp)

define_module_example(NAME pub_example SOURCES topic_example.h pub_example.cpp)

define_module_example(NAME sub_example SOURCES topic_example.h sub_example.cpp)

define_module_example(
  NAME perf_pub
  SOURCES perf_pub.cpp perf_constants.h
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME perf_sub
  SOURCES perf_sub.cpp perf_constants.h
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(NAME service_example SOURCES service_example.cpp)
