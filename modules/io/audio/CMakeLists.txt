# =================================================================================================
# Copyright (C) 2026 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME audio
  DEPENDS_ON_MODULES "conio;log"
  DEPENDS_ON_EXTERNAL_PROJECTS "SDL3")

find_package(SDL3 ${SDL3_VERSION_REQUIRED} REQUIRED)

set(HEADERS
    include/grape/audio/device_info.h include/grape/audio/audio_frame.h
    include/grape/audio/audio_spec.h include/grape/audio/playback.h include/grape/audio/recorder.h)

set(SOURCES src/sdl_conversions.h src/sdl_conversions.cpp src/device_info.cpp src/playback.cpp
            src/recorder.cpp)

define_module_library(
  NAME audio
  SOURCES ${SOURCES}
  PUBLIC_HEADERS ${HEADERS}
  PUBLIC_LINK_LIBS "grape::log;SDL3::SDL3")

add_subdirectory(apps)
