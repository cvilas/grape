# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

define_module_app(
  NAME camera_view
  SOURCES view.cpp
  PUBLIC_LINK_LIBS "grape::log;grape::conio;SDL3::SDL3")
