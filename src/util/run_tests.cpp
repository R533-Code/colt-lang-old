#include "run_tests.h"
#include "args.h"

namespace clt
{
#ifdef COLT_DEBUG
  void run_tests() noexcept
  {

  }
#else
  void run_tests() noexcept
  {
    if (RunTests)
      io::print_warning("Tests can only be run on Debug configuration!");
  }
#endif
}