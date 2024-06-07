/*****************************************************************//**
 * @file   run_tests.h
 * @brief  Contains 'run_tests()' function.
 * 
 * @author RPC
 * @date   February 2024
 *********************************************************************/
#ifndef HG_COLT_RUN_TESTS
#define HG_COLT_RUN_TESTS

#include "io/print.h"
#include "test/test_lexer.h"
#include "test/test_ffi.h"

namespace clt
{
  /// @brief Run all unit tests on Debug configuration,
  /// or prints a warning on Release.
  void run_tests() noexcept;
}

#endif // !HG_COLT_RUN_TESTS
