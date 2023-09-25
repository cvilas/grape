//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#pragma once

namespace grape::utils {

/// @return true if key was pressed on the terminal console. use getch() to read the key stroke.
auto kbhit() -> bool;

/// @return Code for the last keypress from the terminal console
auto getch() -> int;

}  // namespace grape::utils
