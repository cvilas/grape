//=================================================================================================
// Copyright (C) 2018 GRAPE Contributors
//=================================================================================================

#include "grape/conio/conio.h"

#include <cstdio>

#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

namespace grape::conio {

//-------------------------------------------------------------------------------------------------
auto kbhit() -> bool {
  /// reference: http://cboard.cprogramming.com/c-programming/63166-kbhit-linux.html
  timeval tv{};
  fd_set fds{};
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  // NOLINTNEXTLINE(hicpp-no-assembler,readability-isolate-declaration,cppcoreguidelines-pro-bounds-constant-array-index)
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);  // NOLINT(hicpp-signed-bitwise)
  select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv);
  return FD_ISSET(STDIN_FILENO, &fds);  // NOLINT(hicpp-signed-bitwise)
}

//-------------------------------------------------------------------------------------------------
auto getch() -> int {
  /// reference:
  /// https://stackoverflow.com/questions/3276546/how-to-implement-getch-function-of-c-in-linux
  struct termios oldt = {};
  tcgetattr(STDIN_FILENO, &oldt);  // grab old terminal io settings
  struct termios newt = oldt;
  // disable buffered io
  newt.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));  // NOLINT(hicpp-signed-bitwise)
  tcsetattr(STDERR_FILENO, TCSANOW, &newt);                 // new settings set
  const int ch = getchar();
  tcsetattr(STDERR_FILENO, TCSANOW, &oldt);  // terminal settings restored
  return ch;
}

}  // namespace grape::conio