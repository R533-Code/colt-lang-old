/*****************************************************************//**
 * @file   test_lexer.h
 * @brief  Tests for Lexer.
 * 
 * @author RPC
 * @date   February 2024
 *********************************************************************/
#ifndef HG_COLT_TEST_LEXER
#define HG_COLT_TEST_LEXER

namespace clt::test
{
  void test_lexer(StringView file_path, u32& error_count) noexcept;
}

#endif // !HG_COLT_TEST_LEXER
