# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

define_module_app(
  NAME camera_publisher
  SOURCES camera_publisher.cpp
  PUBLIC_LINK_LIBS grape::log grape::ipc grape::serdes SDL3::SDL3)

define_module_app(
  NAME camera_subscriber
  SOURCES camera_subscriber.cpp
  PUBLIC_LINK_LIBS grape::log grape::ipc grape::serdes SDL3::SDL3)
