# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME camera
  DEPENDS_ON_MODULES "log;conio"
  DEPENDS_ON_EXTERNAL_PROJECTS "SDL3")

find_package(SDL3 ${SDL3_VERSION_REQUIRED} REQUIRED)

add_subdirectory(apps)
