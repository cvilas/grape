//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#ifdef __clang__
#if __has_cpp_attribute(clang::lifetimebound)
#define GRAPE_LIFETIMEBOUND [[clang::lifetimebound]]
#else
#define GRAPE_LIFETIMEBOUND
#endif
#else
#define GRAPE_LIFETIMEBOUND
#endif
