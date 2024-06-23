/*****************************************************************/ /**
 * @file   test_lexer.h
 * @brief  Tests for Lexer.
 *
 * @author RPC
 * @date   February 2024
 *********************************************************************/
#ifndef HG_COLT_TEST_LEXER
#define HG_COLT_TEST_LEXER

#include "lex/colt_lexer.h"
#include "err/composable_reporter.h"

namespace clt::test
{
  /// @brief Tests the lexer using a file.
  /// The file should follow a specific format:
  /// Starts with the expected tokens on a line (without TKN_EOF), followed
  /// by the line to lex and compare against the previous line.
  /// Lines starting with a '#' are ignored (useful for comments).
  /// @param file_path The file to use as a test
  /// @param error_count The error count to increment on errors
  void test_lexer(StringView file_path, u32& error_count) noexcept;
} // namespace clt::test

#endif // !HG_COLT_TEST_LEXER
