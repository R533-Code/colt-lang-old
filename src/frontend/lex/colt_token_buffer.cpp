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

  TokenBuffer Lex(ErrorReporter& reporter, StringView to_parse) noexcept
  {
    TokenBuffer buffer;
    Lex(buffer, reporter, to_parse);
    return buffer;
  }

  void Lex(TokenBuffer& buffer, ErrorReporter& reporter, StringView to_parse) noexcept
  {
    CreateLines(to_parse, buffer);
    Lexer lex = { reporter, buffer };   
    
    lex.next = lex.getNext();
    while (lex.next != EOF)
      Lexer::LexingTable[(u8)lex.next](lex);
    // Add EOF (even if there is already an EOF)
    if (buffer.getTokens().is_empty())
      buffer.addToken(Lexeme::TKN_EOF, 0, 0, 0);
    else
    {
      auto& token = buffer.getTokens().back();
      buffer.addToken(Lexeme::TKN_EOF, buffer.getLine(token) - 1, buffer.getColumn(token) + 1, 0);
    }
  }

  void CreateLines(StringView strv, TokenBuffer& buffer) noexcept
  {
    const auto front = strv.data();
    const char* const text = strv.data();
    const i64 size = strv.size();
    i64 start = 0;
    while (auto nl = static_cast<const char*>(memchr(&text[start], '\n', size - start)))
    {
      i64 nl_index = nl - text;
      // + 1 to include the '\n'
      buffer.addLine(StringView{ front + start, static_cast<size_t>(nl_index - start) + 1 });
      start = nl_index + 1;
    }
    // The last line ends at the end of the file.
    buffer.addLine(StringView{ front + start, static_cast<size_t>(size - start) });
    /*if (start != size)
      buffer.push_back(StringView{ front + start, 0 });*/
  }
}