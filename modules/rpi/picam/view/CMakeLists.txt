# =================================================================================================
# Copyright (C) 2025 grape contributors
# =================================================================================================

find_package(SDL3 ${SDL3_VERSION_REQUIRED} REQUIRED)

set(SOURCES
  main.cpp
)
define_module_app(NAME ${MODULE_NAME}_view SOURCES ${SOURCES} PUBLIC_LINK_LIBS PkgConfig::LIBCAMERA grape::log SDL3::SDL3)
