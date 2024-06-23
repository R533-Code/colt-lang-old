/*****************************************************************/ /**
 * @file   test_lexer.cpp
 * @brief  Implementation of 'test_lexer'.
 *
 * @author RPC
 * @date   June 2024
 *********************************************************************/
#include "test_lexer.h"
#include "lex/colt_token_buffer.h"
#include "err/composable_reporter.h"
#include <fstream>

namespace clt::test
{
  void test_lexer(StringView file_path, u32& error_count) noexcept
  {
    io::print_message("Testing Lexer...");

    using namespace lng;

    auto reporter = make_error_reporter<SinkReporter>();

    std::string str = {file_path.data(), file_path.size()};
    std::ifstream is(str);
    if (!is.good())
    {
      error_count++;
      return io::print_error("Could not open file '{}'!", str);
    }

    // The line number in the file to report eventual errors
    u64 true_line_nb = 0;

    bool consume_line = false;
    // When multiple of 2, we are parsing the expected lexemes
    // Else, we are parsing the string to lex.
    u64 line_count = 0;
    TokenBuffer buffer;
    Vector<Lexeme> expected_lexemes{};
    while (std::getline(is, str))
    {
      true_line_nb++;

      // Skip lines starting with '#'
      if (strip(str).starts_with("#"))
        continue;
      if (line_count % 2 == 0)
      {
        for (auto tkn_str : split_by_char(str, ' '))
        {
          auto try_cnv = reflect<Lexeme>::from(tkn_str);
          if (try_cnv.is_none())
          {
            consume_line = true;
            error_count++;
            io::print_error(
                "'{}' is not a valid lexeme (on line {}).", tkn_str, true_line_nb);
            goto WHILE_END;
          }
          expected_lexemes.push_back(*try_cnv);
        }
        // add EOF
        expected_lexemes.push_back(Lexeme::TKN_EOF);
      }
      else
      {
        // There was an error parsing expected lexemes so skip
        if (!consume_line)
        {
          lex(buffer, *reporter, str);
          for (size_t i = 0;
               i < clt::min(buffer.token_buffer().size(), expected_lexemes.size());
               i++)
          {
            if (expected_lexemes[i] != buffer.token_buffer()[i])
            {
              error_count++;
              io::print_error(
                  "Expected '{:h}' but Lexer returned '{:h}' instead (on line {})!",
                  expected_lexemes[i], buffer.token_buffer()[i].lexeme(),
                  true_line_nb);
            }
          }
          if (buffer.token_buffer().size() != expected_lexemes.size())
          {
            error_count++;
            io::print_error(
                "Expected '{}' lexemes but Lexer returned '{}' instead (on line "
                "{})!",
                expected_lexemes.size(), buffer.token_buffer().size(), true_line_nb);
          }
          // To avoid constructing a TokenBuffer in each iteration
          buffer.unsafe_clear();
        }
        else
          consume_line = false;
        expected_lexemes.clear();
      }
    WHILE_END:
      ++line_count;
    }
  }
} // namespace clt::test