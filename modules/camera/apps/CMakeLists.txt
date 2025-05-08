# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

find_package(SDL3 ${SDL3_VERSION_REQUIRED} REQUIRED)
find_package(lz4 ${LZ4_VERSION_REQUIRED} REQUIRED)

define_module_app(NAME camera_view SOURCES view.cpp PUBLIC_LINK_LIBS grape::log SDL3::SDL3)
define_module_app(NAME camera_pub SOURCES pub.cpp pub_sub.h PUBLIC_LINK_LIBS grape::ipc grape::serdes grape::log SDL3::SDL3)
define_module_app(NAME camera_sub SOURCES sub.cpp pub_sub.h PUBLIC_LINK_LIBS grape::ipc grape::serdes grape::log SDL3::SDL3)
