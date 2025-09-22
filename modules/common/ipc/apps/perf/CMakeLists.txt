# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

define_module_app(
  NAME ipc_perf_pub
  SOURCES perf_pub.cpp perf_constants.h
  PUBLIC_LINK_LIBS grape::conio)

define_module_app(
  NAME ipc_perf_sub
  SOURCES perf_sub.cpp perf_constants.h
  PUBLIC_LINK_LIBS grape::conio)
