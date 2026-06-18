# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME realtime
  DEPENDS_ON_MODULES "base;conio"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(HEADERS
    include/grape/realtime/mpsc_queue.h include/grape/realtime/spmcq.h
    include/grape/realtime/mutex.h include/grape/realtime/schedule.h
    include/grape/realtime/thread.h)

set(SOURCES src/schedule.cpp src/spmcq.cpp)

# library target
define_module_library(
  NAME realtime
  PUBLIC_LINK_LIBS "grape::base;grape::conio"
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_HEADERS ${HEADERS}
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(tests)
add_subdirectory(examples)
add_subdirectory(apps)
