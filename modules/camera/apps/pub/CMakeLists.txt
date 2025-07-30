# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

define_module_app(
  NAME camera_pub
  SOURCES camera_pub.cpp
  PUBLIC_LINK_LIBS "grape::conio")
