# =================================================================================================
# Copyright (C) 2025 grape contributors
# =================================================================================================

set(SOURCES list.cpp)

define_module_app(
  NAME ${MODULE_NAME}_list
  SOURCES ${SOURCES}
  PUBLIC_LINK_LIBS PkgConfig::LIBCAMERA grape::log)
