# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME joystick
  DEPENDS_ON_MODULES "base"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  message(STATUS "${MODULE_NAME}: Only supported on Linux. Bypassing module.")
  return()
endif()

set(SOURCES README.md include/grape/joystick/controls.h include/grape/joystick/device_info.h
            include/grape/joystick/events.h include/grape/joystick/joystick.h src/joystick.cpp)

define_module_library(
  NAME joystick
  PUBLIC_LINK_LIBS "grape::base"
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

add_subdirectory(examples)
