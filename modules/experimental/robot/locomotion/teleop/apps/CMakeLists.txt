# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

find_package(ftxui ${FTXUI_VERSION_REQUIRED} REQUIRED)

define_module_app(
  NAME keyboard_teleop
  SOURCES keyboard_teleop.cpp
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS "grape::conio;grape::teleop_client;ftxui::screen;ftxui::dom;ftxui::component")
