# =================================================================================================
# Copyright (C) 2026 GRAPE Contributors
# =================================================================================================

define_module_app(
  NAME audio_monitor
  SOURCES monitor.cpp
  PUBLIC_LINK_LIBS "grape::audio;grape::conio;grape::log;SDL3::SDL3")
