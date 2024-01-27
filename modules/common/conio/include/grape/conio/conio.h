//=================================================================================================
// Copyright (C) 2018 GRAPE Contributors
//=================================================================================================

#pragma once

namespace grape::conio {

/// @return true if key was pressed on the terminal console. use getch() to read the key stroke.
auto kbhit() -> bool;

/// @return Code for the last keypress from the terminal console without blocking
auto getch() -> int;

}  // namespace grape::conio
