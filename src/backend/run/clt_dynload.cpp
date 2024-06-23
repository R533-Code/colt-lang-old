#include "clt_dynload.h"

#ifndef _WIN32
  #include <limits.h>
  #include <unistd.h> //readlink
#endif

namespace clt::run
{
  Option<DynamicLibrary> DynamicLibrary::load(const char* path) noexcept
  {
    assert_true("Invalid path!", path != nullptr);
    auto lib  = dlLoadLibrary(path);
    auto syms = dlSymsInit(path);
    if (lib == nullptr || syms == nullptr)
      return None;
    return DynamicLibrary(lib, syms);
  }
  
  Option<DynamicLibrary> DynamicLibrary::load_current() noexcept
  {
#ifdef _WIN32
    auto lib  = dlLoadLibrary(nullptr);
    auto syms = dlSymsInit(nullptr);
    if (lib == nullptr || syms == nullptr)
      return None;
    return DynamicLibrary(lib, syms);
#else
    char result[PATH_MAX] = {0};
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count < 0)
      return None;
    auto lib  = dlLoadLibrary(nullptr);
    auto syms = dlSymsInit(result);
    if (lib == nullptr || syms == nullptr)
      return None;
    return DynamicLibrary(lib, syms);
#endif
  }
}

