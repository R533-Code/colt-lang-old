/*****************************************************************//**
 * @file   test_lexer.h
 * @brief  Tests for FFI.
 *
 * @author RPC
 * @date   February 2024
 *********************************************************************/
#ifndef HG_COLT_TEST_FFI
#define HG_COLT_TEST_FFI

#include "run/clt_dyncall.h"

namespace clt::test
{
  /// @brief Tests the FFI used by the interpreter.
  /// @param error_count The error count to increment on errors
  void test_ffi(u32& error_count) noexcept;
}

#endif // !HG_COLT_TEST_FFI
