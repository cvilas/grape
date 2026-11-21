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

/// Forces a function to be inlined regardless of the compiler's size heuristics. For small helpers
/// whose call overhead (e.g. returning a large struct through memory) outweighs their body.
#define GRAPE_ALWAYS_INLINE [[gnu::always_inline]]
