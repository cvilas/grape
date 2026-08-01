# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

declare_module(NAME utils NO_MODULE_DEPENDENCIES)

# library sources
set(HEADERS
    include/grape/utils/detail/enums_detail.h include/grape/utils/enums.h
    include/grape/utils/file_system.h include/grape/utils/ip.h include/grape/utils/stacktrace.h
    include/grape/utils/utils.h)

set(SOURCES src/ip.cpp src/file_system.cpp src/stacktrace.cpp src/utils.cpp)

# library target
define_module_library(
  NAME utils
  PUBLIC_LINK_LIBS ""
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_HEADERS ${HEADERS}
  PRIVATE_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(examples)
add_subdirectory(tests)
add_subdirectory(apps)
