# =================================================================================================
# Copyright (C) 2026 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME rpi_sense_hat
  DEPENDS_ON_MODULES "realtime"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

set(SOURCES
    README.md
    include/grape/rpi/sense_hat/display.h
    include/grape/rpi/sense_hat/humidity_sensor.h
    include/grape/rpi/sense_hat/pressure_sensor.h
    include/grape/rpi/sense_hat/i2c_bus.h
    include/grape/rpi/sense_hat/inertial_sensor.h
    src/display.cpp
    src/humidity_sensor.cpp
    src/pressure_sensor.cpp
    src/i2c_bus.cpp
    src/hts221.h
    src/lps25h.h
    src/lsm9ds1.h
    src/utils.h
    src/inertial_sensor.cpp)

define_module_library(
  NAME rpi_sense_hat
  PUBLIC_LINK_LIBS "grape::base;grape::realtime"
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

add_subdirectory(examples)
