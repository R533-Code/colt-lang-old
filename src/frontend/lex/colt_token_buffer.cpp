/*****************************************************************//**
 * @file   colt_token_buffer.cpp
 * @brief  Contains the implementation of 'colt_token_buffer.h'.
 * 
 * @author RPC
 * @date   April 2024
 *********************************************************************/
#include "colt_token_buffer.h"
#include "colt_lexer.h"

namespace clt::lng
{
#ifdef COLT_DEBUG
  std::atomic<u32> TokenBuffer::ID_GENERATOR{};
#endif // COLT_DEBUG

  TokenBuffer lex(ErrorReporter& reporter, StringView to_parse) noexcept
  {
    TokenBuffer buffer;
    lex(buffer, reporter, to_parse);
    return buffer;
  }

  void lex(TokenBuffer& buffer, ErrorReporter& reporter, StringView to_parse) noexcept
  {
    create_lines(to_parse, buffer);
    Lexer lex = { reporter, buffer };   
    
    lex._next = lex.next();
    while (lex._next != EOF)
      Lexer::LexingTable[(u8)lex._next](lex);
    // Add EOF (even if there is already an EOF)
    if (buffer.token_buffer().is_empty())
      buffer.add_token(Lexeme::TKN_EOF, 0, 0, 0);
    else
    {
      auto& token = buffer.token_buffer().back();
      buffer.add_token(Lexeme::TKN_EOF, buffer.line_nb(token) - 1, buffer.column_nb(token) + 1, 0);
    }
  }

  void create_lines(StringView strv, TokenBuffer& buffer) noexcept
  {
    const auto front = strv.data();
    const char* const text = strv.data();
    const i64 size = strv.size();
    i64 start = 0;
    while (auto nl = static_cast<const char*>(memchr(&text[start], '\n', size - start)))
    {
      i64 nl_index = nl - text;
      // + 1 to include the '\n'
      buffer.add_line(StringView{ front + start, static_cast<size_t>(nl_index - start) + 1 });
      start = nl_index + 1;
    }
    // The last line ends at the end of the file.
    buffer.add_line(StringView{ front + start, static_cast<size_t>(size - start) });
    /*if (start != size)
      buffer.push_back(StringView{ front + start, 0 });*/
  }
}