#include "colt_lexer.h"

namespace clt::lng
{
  /// @brief Consumes multi-line comments recursively.
  /// This function can throw an ExitRecursionExcept.
  /// @param lexer The lexer used to parse
  static void consume_lines_comment_throw(Lexer& lexer);

  void ConsumeTillWhitespaces(Lexer& lexer) noexcept
  {
    // Consume till a whitespace or EOF is hit
    while (!clt::isspace(lexer.next) && lexer.next != EOF)
      lexer.next = lexer.getNext();
  }

  void ConsumeWhitespaces(Lexer& lexer) noexcept
  {
    // Consume while whitespace and not EOF is hit
    while (clt::isspace(lexer.next) && lexer.next != EOF)
      lexer.next = lexer.getNext();
  }

  static void consume_lines_comment_throw(Lexer& lexer)
  {
    const u32 line_nb = lexer.line_nb;

    if (lexer.comment_depth == std::numeric_limits<u8>::max())
    {
      lexer.reporter.error("Exceeded recursion depth!");
      lexer.next = EOF; // To stop parsing
      throw ExitRecursionExcept{};
    }
    lexer.comment_depth++;
    assert_true("Invalid call to ConsumeLinesComment!", lexer.offset >= Lexer::MultilineCommentSize);
    const auto start_offset = lexer.offset - Lexer::MultilineCommentSize - 1;

    do
    {
      // We hit a nested comment
      if (lexer.next == '/' && lexer.peekNext() == '*')
      {
        lexer.getNext(); // consume '/'
        lexer.next = lexer.getNext(); // consume '*'
        consume_lines_comment_throw(lexer);
        continue;
      }
      if (lexer.next == '*' && lexer.peekNext() == '/')
      {
        lexer.getNext(); // consume '/'
        lexer.next = lexer.getNext();
        return;
      }
      lexer.next = lexer.getNext();
    } while (lexer.next != EOF);

    // We hit EOF
    lexer.reporter.error("Unterminated multi-line comment!",
      lexer.makeSource(line_nb, start_offset, start_offset + Lexer::MultilineCommentSize));
    throw ExitRecursionExcept{};
  }

  void ConsumeLinesComment(Lexer& lexer) noexcept
  {
    try
    {
      consume_lines_comment_throw(lexer);
    }
    catch (...)
    {
      // We catch the exception:
      // consume_lines_comment_throw will have reported the exception.
      // This allows all other functions to be noexcept.
    }
  }

  void ConsumeDigits(Lexer& lexer) noexcept
  {
    // Consume while is digit and not EOF is hit
    while (clt::isdigit(lexer.next) && lexer.next != EOF)
    {
      lexer.temp.push_back(lexer.next);
      lexer.next = lexer.getNext();
    }
  }

  void ParseInvalid(Lexer& lexer) noexcept
  {
    auto snap = lexer.startLexeme();

    ConsumeTillWhitespaces(lexer);
    lexer.addToken(Lexeme::TKN_ERROR, snap);

    // TODO: add error number
    lexer.reporter.error("Invalid character!", lexer.makeSource(snap));
  }

  void ParsePlus(Lexer& lexer) noexcept
  {
    auto snap = lexer.startLexeme();

    lexer.next = lexer.getNext();
    switch (lexer.next)
    {
    break; case '=':
      lexer.next = lexer.getNext(); // consume '='
      lexer.addToken(Lexeme::TKN_PLUS_EQUAL, snap);
    break; case '+':
      lexer.next = lexer.getNext(); // consume '+'
      lexer.addToken(Lexeme::TKN_PLUS_PLUS, snap);
    break; default:
      lexer.addToken(Lexeme::TKN_PLUS, snap);
    }
  }

  void ParseMinus(Lexer& lexer) noexcept
  {
    auto snap = lexer.startLexeme();

    lexer.next = lexer.getNext();
    switch (lexer.next)
    {
    break; case '=':
      lexer.next = lexer.getNext(); // consume '='
      lexer.addToken(Lexeme::TKN_MINUS_EQUAL, snap);
    break; case '-':
      lexer.next = lexer.getNext(); // consume '-'
      lexer.addToken(Lexeme::TKN_MINUS_MINUS, snap);
    break; default:
      lexer.addToken(Lexeme::TKN_MINUS, snap);
    }
  }

  void ParseStar(Lexer& lexer) noexcept
  {
    auto snap = lexer.startLexeme();

    lexer.next = lexer.getNext();
    if (lexer.next == '=')
    {
      lexer.next = lexer.getNext(); // consume '='
      lexer.addToken(Lexeme::TKN_STAR_EQUAL, snap);
    }
    else
      lexer.addToken(Lexeme::TKN_STAR, snap);
  }

  void ParseSlash(Lexer& lexer) noexcept
  {
    auto snap = lexer.startLexeme();

    lexer.next = lexer.getNext();
    switch (lexer.next)
    {
    break; case '=':
      lexer.next = lexer.getNext(); // consume '='
      lexer.addToken(Lexeme::TKN_SLASH_EQUAL, snap);

      /****  COMMENTS HANDLING  ****/
    break; case '/':
      // Go to next line
      lexer.line_nb++;
      lexer.offset = 0;
    break; case '*':
      lexer.next = lexer.getNext(); // consume '*'
      ConsumeLinesComment(lexer);
    break; default:
      lexer.addToken(Lexeme::TKN_SLASH, snap);
    }
  }

  void ParsePercent(Lexer& lexer) noexcept
  {
    auto snap = lexer.startLexeme();

    lexer.next = lexer.getNext();
    if (lexer.next == '=')
    {
      lexer.next = lexer.getNext(); // consume '='
      lexer.addToken(Lexeme::TKN_PERCENT_EQUAL, snap);
    }
    else
      lexer.addToken(Lexeme::TKN_PERCENT, snap);
  }

  void ParseColon(Lexer& lexer) noexcept
  {
    auto snap = lexer.startLexeme();

    lexer.next = lexer.getNext();
    if (lexer.next == ':')
    {
      lexer.next = lexer.getNext(); // consume ':'
      lexer.addToken(Lexeme::TKN_COLON_COLON, snap);
    }
    else
      lexer.addToken(Lexeme::TKN_COLON, snap);
  }

  void ParseEqual(Lexer& lexer) noexcept
  {
    auto snap = lexer.startLexeme();

    lexer.next = lexer.getNext();
    switch (lexer.next)
    {
    break; case '=':
      lexer.next = lexer.getNext(); // consume '='
      lexer.addToken(Lexeme::TKN_EQUAL_EQUAL, snap);
    break; case '>':
      lexer.next = lexer.getNext(); // consume '>'
      lexer.addToken(Lexeme::TKN_EQUAL_GREAT, snap);
    break; default:
      lexer.addToken(Lexeme::TKN_EQUAL, snap);
    }
  }

  void ParseExclam(Lexer& lexer) noexcept
  {
    auto snap = lexer.startLexeme();

    lexer.next = lexer.getNext();
    if (lexer.next == '=')
    {
      lexer.next = lexer.getNext(); // consume '='
      lexer.addToken(Lexeme::TKN_EXCLAM_EQUAL, snap);
    }
    else
      lexer.addToken(Lexeme::TKN_EXCLAM, snap);
  }

  void ParseLt(Lexer& lexer) noexcept
  {
    auto snap = lexer.startLexeme();

    lexer.next = lexer.getNext();
    switch (lexer.next)
    {
    break; case '=':
      lexer.next = lexer.getNext(); // consume '='
      lexer.addToken(Lexeme::TKN_LESS_EQUAL, snap);

    break; case '<':
      lexer.next = lexer.getNext(); // consume '<'
      if (lexer.next == '=')
      {
        lexer.next = lexer.getNext(); // consume '='
        lexer.addToken(Lexeme::TKN_LESS_LESS_EQUAL, snap);
      }
      else
        lexer.addToken(Lexeme::TKN_LESS_LESS, snap);

    break; default:
      lexer.addToken(Lexeme::TKN_LESS, snap);
    }
  }

  void ParseGt(Lexer& lexer) noexcept
  {
    auto snap = lexer.startLexeme();

    lexer.next = lexer.getNext();
    switch (lexer.next)
    {
    break; case '=':
      lexer.next = lexer.getNext(); // consume '='
      lexer.addToken(Lexeme::TKN_GREAT_EQUAL, snap);

    break; case '>':
      lexer.next = lexer.getNext(); // consume '>'
      if (lexer.next == '=')
      {
        lexer.next = lexer.getNext(); // consume '='
        lexer.addToken(Lexeme::TKN_GREAT_GREAT_EQUAL, snap);
      }
      else
        lexer.addToken(Lexeme::TKN_GREAT_GREAT, snap);

    break; default:
      lexer.addToken(Lexeme::TKN_GREAT, snap);
    }
  }

  void ParseAnd(Lexer& lexer) noexcept
  {
    auto snap = lexer.startLexeme();

    lexer.next = lexer.getNext();
    switch (lexer.next)
    {
    break; case '=':
      lexer.next = lexer.getNext(); // consume '='
      lexer.addToken(Lexeme::TKN_AND_EQUAL, snap);
    break; case '&':
      lexer.next = lexer.getNext(); // consume '&'
      lexer.addToken(Lexeme::TKN_AND_AND, snap);
    break; default:
      lexer.addToken(Lexeme::TKN_AND, snap);
    }
  }

  void ParseOr(Lexer& lexer) noexcept
  {
    auto snap = lexer.startLexeme();

    lexer.next = lexer.getNext();
    switch (lexer.next)
    {
    break; case '=':
      lexer.next = lexer.getNext(); // consume '='
      lexer.addToken(Lexeme::TKN_OR_EQUAL, snap);
    break; case '&':
      lexer.next = lexer.getNext(); // consume '&'
      lexer.addToken(Lexeme::TKN_OR_OR, snap);
    break; default:
      lexer.addToken(Lexeme::TKN_OR, snap);
    }
  }

  void ParseCaret(Lexer& lexer) noexcept
  {
    auto snap = lexer.startLexeme();

    lexer.next = lexer.getNext();
    if (lexer.next == '=')
    {
      lexer.next = lexer.getNext(); // consume '='
      lexer.addToken(Lexeme::TKN_CARET_EQUAL, snap);
    }
    else
      lexer.addToken(Lexeme::TKN_CARET, snap);
  }

  void parse_float(Lexer& lexer, const Lexer::Snapshot& snap) noexcept
  {
    float value;
    auto result = clt::parse(lexer.temp, value);
    if (result.code() == ParsingCode::GOOD)
      return lexer.addLiteral(Lexeme::TKN_DOUBLE_L, value, snap);

    lexer.addToken(Lexeme::TKN_ERROR, snap);
    lexer.reporter.error("Invalid 'f32' literal!",
      lexer.makeSource(snap));
  }

  void parse_double(Lexer& lexer, const Lexer::Snapshot& snap) noexcept
  {
    double value;
    auto result = clt::parse(lexer.temp, value);
    if (result.code() == ParsingCode::GOOD)
      return lexer.addLiteral(Lexeme::TKN_DOUBLE_L, value, snap);

    lexer.addToken(Lexeme::TKN_ERROR, snap);
    lexer.reporter.error("Invalid 'f64' literal!",
      lexer.makeSource(snap));
  }

  void ParseDot(Lexer& lexer) noexcept
  {
    auto snap = lexer.startLexeme();

    lexer.next = lexer.getNext();
    if (!clt::isdigit(lexer.next))
      return lexer.addToken(Lexeme::TKN_DOT, snap);

    //Clear the string
    lexer.temp.clear();
    lexer.temp.push_back('.');
    ConsumeDigits(lexer);

    // We are parsing an exponent
    if (lexer.next == 'e')
    {
      char after_e = lexer.peekNext();
      if (clt::isdigit(after_e))
      {
        lexer.next = lexer.getNext(); // consume 'e'
        lexer.temp.push_back('e');
        ConsumeDigits(lexer);
      }
      else if (after_e == '+' && clt::isdigit(lexer.peekNext(1)))
      {
        lexer.getNext(); // consume 'e'
        lexer.next = lexer.getNext(); // consume '+'
        lexer.temp.push_back('e');
        ConsumeDigits(lexer);
      }
      else if (after_e == '-' && clt::isdigit(lexer.peekNext(1)))
      {
        lexer.getNext(); // consume 'e'
        lexer.next = lexer.getNext(); // consume '-'
        lexer.temp.push_back("e-");
        ConsumeDigits(lexer);
      }
    }

    if (lexer.next == 'f')
    {
      lexer.next = lexer.getNext();
      return parse_float(lexer, snap);
    }
    if (lexer.next == 'd')
      lexer.next = lexer.getNext();
    parse_double(lexer, snap);
  }
}