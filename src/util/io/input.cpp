#include "common/colt_config.h"

#ifndef COLT_WINDOWS
  #include <termios.h>
  #include <unistd.h>
#else
#define NOMINMAX
  #include <Windows.h>
#endif //COLT_WINDOWS

#include <cstdio>

namespace clt::io
{
  void toggle_echo() noexcept
  {
#ifdef _WIN32
    HANDLE h;
    DWORD mode;

    h = GetStdHandle(STD_INPUT_HANDLE);
    if (GetConsoleMode(h, &mode))
    {
      mode &= ~ENABLE_ECHO_INPUT;
      SetConsoleMode(h, mode);
    }
#else
    struct termios ts;
    tcgetattr(fileno(stdin), &ts);
    ts.c_lflag &= ~ECHO;
    tcsetattr(fileno(stdin), TCSANOW, &ts);
#endif
  }
}