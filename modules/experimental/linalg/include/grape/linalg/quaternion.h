//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <type_traits>

namespace grape::linalg {

template<std::floating_point ScalarType = double>
struct Quaternion {
    ScalarType x;
    ScalarType y;
    ScalarType z;
    ScalarType w;
};

}