# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

define_module_app(
  NAME camera_pipeline
  SOURCES pipeline.cpp
  PUBLIC_LINK_LIBS "grape::conio")
