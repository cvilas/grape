# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME camera
  DEPENDS_ON_MODULES "log;ipc;serdes"
  DEPENDS_ON_EXTERNAL_PROJECTS "SDL3;lz4")
  
  add_subdirectory(apps)