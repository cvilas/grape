# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

find_package(SDL3 ${SDL3_VERSION_REQUIRED} REQUIRED)
find_package(lz4 ${LZ4_VERSION_REQUIRED} REQUIRED)

define_module_app(NAME camera_view SOURCES view.cpp PUBLIC_LINK_LIBS grape::log SDL3::SDL3)
set_target_properties(grape_camera_view PROPERTIES COMPILE_OPTIONS "${THIRD_PARTY_COMPILER_WARNINGS}")
