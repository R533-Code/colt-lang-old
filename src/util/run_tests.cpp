#include "run_tests.h"
#include "args.h"
#include "lex/test_lexer.h"

namespace clt
{
#ifdef COLT_DEBUG
  void run_tests() noexcept
  {
    if (!RunTests)
      return;

    u32 error_count = 0;
    u32 run_test_count = 0;
    
    if (!LexerTestFile.empty())
    {
      ++run_test_count;
      test::test_lexer(LexerTestFile, error_count);
    }

    if (run_test_count == 0)
    {
      io::print_warn("{}-run-tests{} was specified but no tests were run!", io::BrightCyanF, io::Reset);
      io::print_message("As an example, use {}-test-lexer{}={}<FILEPATH>{} to test the lexer.", io::BrightCyanF, io::Reset, io::BrightMagentaF, io::Reset);
      return;
    }
    io::print_message("Tested {} features with {} error{}", run_test_count, error_count,
      error_count ? "s." : ".");
  }
#else
  void run_tests() noexcept
  {
    if (RunTests)
      io::print_warning("Tests can only be run on Debug configuration!");
  }
#endif
}