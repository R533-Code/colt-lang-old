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

  void ConsumeTillSpaceOrPunct(Lexer& lexer) noexcept
  {
    // Consume till a whitespace or a punctuation or EOF is hit
    while (!clt::isspace(lexer.next) && !clt::ispunct(lexer.next) && lexer.next != EOF)
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

  void ConsumeDigits(Lexer& lexer, int base) noexcept
  {
    assert_true("Invalid base!", base > 1, base <= 16);
    if (base <= 10)
    {
      while (('0' <= lexer.next && lexer.next < '0' + base) && lexer.next != EOF)
      {
        lexer.temp.push_back(lexer.next);
        lexer.next = lexer.getNext();
      }
    }
    else
    {
      const int minus_10 = base - 10;
      while ((('0' <= lexer.next && lexer.next < '0' + base)
        || ('A' <= clt::toupper(lexer.next) &&  clt::toupper(lexer.next) < 'A' + minus_10))
        && lexer.next != EOF)
      {
        lexer.temp.push_back(lexer.next);
        lexer.next = lexer.getNext();
      }
    }
  }

  void ConsumeAlnum(Lexer& lexer) noexcept
  {
    // Consume while is alnum and not EOF is hit
    while (clt::isalnum(lexer.next) && lexer.next != EOF)
    {
      lexer.temp.push_back(lexer.next);
      lexer.next = lexer.getNext();
    }
  }

  void ParseInvalid(Lexer& lexer) noexcept
  {
    auto snap = lexer.startLexeme();

    lexer.next = lexer.getNext();
    ConsumeTillSpaceOrPunct(lexer);
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

  void ParseDigit(Lexer& lexer) noexcept
  {
    auto snap = lexer.startLexeme();
    lexer.temp.clear();
    lexer.temp.push_back(lexer.next);
    
    if (lexer.next == '0') //Could be 0x, 0b, 0o
    {
      lexer.next = lexer.getNext();
      char symbol = lexer.next;
      int base = 10;
      switch (clt::tolower(symbol))
      {
      break; case 'x': //HEXADECIMAL
        base = 16;
      break; case 'b': //BINARY
        base = 2;
      break; case 'o': //OCTAL
        base = 8;
      break; default:
        //If not any 'x', 'b' or 'o', parse normally
        if (clt::isdigit(symbol) || symbol == '.')
          goto NORM;
        else //If not digit nor '.', then simply '0'
          return HandleIntWithExtension(lexer, snap);
      }
      lexer.next = lexer.getNext(); //Consume symbol
      //Pop the leading '0'
      lexer.temp.clear();
      ConsumeDigits(lexer, base);

      if (lexer.temp.size() == 0) //Contains only the '0'
      {
        const char* range_str;
        switch_no_default(symbol)
        {
        break; case 'x':
          range_str = "Integral literals starting with '0x' should be followed by characters in range [0-9] or [a-f]!";
        break; case 'b':
          range_str = "Integral literals starting with '0b' should be followed by characters in range [0-1]!";
        break; case 'o':
          range_str = "Integral literals starting with '0o' should be followed by characters in range [0-7]!";
        }
        ConsumeTillSpaceOrPunct(lexer);
        lexer.reporter.error(range_str, lexer.makeSource(snap));
        return lexer.addToken(Lexeme::TKN_ERROR, snap);
      }
      return HandleIntWithExtension<true>(lexer, snap, base);
    }
    lexer.next = lexer.getNext();
  NORM:
    //Parse as many digits as possible
    ConsumeDigits(lexer);

    bool isfloat = false;
    // [0-9]+ followed by a .[0-9] is a float
    if (lexer.next == '.')
    {
      //Snapshot for '.' character
      auto snap_dot = lexer.startLexeme();

      lexer.next = lexer.getNext();
      if (clt::isdigit(lexer.next))
      {
        isfloat = true;
        lexer.temp.push_back('.');
        lexer.temp.push_back(lexer.next);
        lexer.next = lexer.getNext();

        //Parse as many digits as possible
        ConsumeDigits(lexer);
      }
      else
      {
        //We parse the integer
        parse_integral<i64, LiteralFromType<i64>()>(lexer, snap);
                
        //The dot is not followed by a digit, this is not a float,
        //but rather should be a dot followed by an identifier for a function call
        lexer.next = lexer.getNext();
        return lexer.addToken(Lexeme::TKN_DOT, snap_dot);
      }
    }

    // [0-9]+(.[0-9]+)?e[+-][0-9]+ is a float
    // We are possibly parsing an exponent
    if (lexer.next == 'e')
    {
      char after_e = lexer.peekNext();
      if (clt::isdigit(after_e))
      {
        isfloat = true;
        lexer.next = lexer.getNext(); // consume 'e'
        lexer.temp.push_back('e');
        ConsumeDigits(lexer);
      }
      else if (after_e == '+' && clt::isdigit(lexer.peekNext(1)))
      {
        isfloat = true;
        lexer.getNext(); // consume 'e'
        lexer.next = lexer.getNext(); // consume '+'
        lexer.temp.push_back('e');
        ConsumeDigits(lexer);
      }
      else if (after_e == '-' && clt::isdigit(lexer.peekNext(1)))
      {
        isfloat = true;
        lexer.getNext(); // consume 'e'
        lexer.next = lexer.getNext(); // consume '-'
        lexer.temp.push_back("e-");
        ConsumeDigits(lexer);
      }
    }

    if (isfloat)
      HandleFloatWithExtension(lexer, snap);
    else
      HandleIntWithExtension(lexer, snap);
  }

  /// @brief Map from keyword string to lexeme
  static constexpr auto KeywordMap = getKeywordMap();

  void ParseIdentifier(Lexer& lexer) noexcept
  {
    auto snap = lexer.startLexeme();

    // Consume till a whitespace or EOF is hit
    while (clt::isalnum(lexer.next) || lexer.next == '_')
      lexer.next = lexer.getNext();
    
    StringView identifier = lexer.getCurrentIdentifier(snap);
    // This is a keyword
    if (auto keyword_opt = KeywordMap.find(identifier); keyword_opt.is_value())
      return lexer.addToken(*keyword_opt, snap);

    if (identifier.starts_with("___"))
    {
      lexer.reporter.error(
        "Identifiers starting with '___' are reserved for the compiler!",
        lexer.makeSource(snap));
      // TODO: add identifier but increment error count
      return lexer.addToken(Lexeme::TKN_ERROR, snap);
    }
    lexer.addIdentifier(identifier, snap);
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

    // We are possibly parsing an exponent
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
    HandleFloatWithExtension(lexer, snap);
  }

  void PrintToken(Token tkn, const TokenBuffer& buffer) noexcept
  {
    using enum Lexeme;
    
    if (isLiteralToken(tkn) && tkn != TKN_STRING_L)
    {
      switch_no_default(tkn.getLexeme())
      {
      case TKN_BOOL_L:
        return io::print("{:h} {}", tkn.getLexeme(), buffer.getLiteral(tkn).as<bool>());
      case TKN_CHAR_L:
        return io::print("{:h} {}", tkn.getLexeme(), buffer.getLiteral(tkn).as<char>());      
      case TKN_U8_L:
      case TKN_U16_L:
      case TKN_U32_L:
      case TKN_U64_L:
        return io::print("{:h} {}", tkn.getLexeme(), buffer.getLiteral(tkn).as<u64>());      
      case TKN_I8_L:
        return io::print("{:h} {}", tkn.getLexeme(), buffer.getLiteral(tkn).as<i8>());      
      case TKN_I16_L:
        return io::print("{:h} {}", tkn.getLexeme(), buffer.getLiteral(tkn).as<i16>());      
      case TKN_I32_L:
        return io::print("{:h} {}", tkn.getLexeme(), buffer.getLiteral(tkn).as<i32>());      
      case TKN_I64_L:
        return io::print("{:h} {}", tkn.getLexeme(), buffer.getLiteral(tkn).as<i64>());
      case TKN_FLOAT_L:
        return io::print("{:h} {}", tkn.getLexeme(), buffer.getLiteral(tkn).as<f32>());      
      case TKN_DOUBLE_L:
        return io::print("{:h} {}", tkn.getLexeme(), buffer.getLiteral(tkn).as<f64>());
      }
    }
    else if (tkn == TKN_IDENTIFIER)
      return io::print("{:h} {}", tkn.getLexeme(), buffer.getIdentifier(tkn));
    return io::print("{:h}", tkn.getLexeme());
  }
  
  void HandleFloatWithExtension(Lexer& lexer, const Lexer::Snapshot& snap) noexcept
  {
    if (lexer.next == 'f')
    {
      lexer.next = lexer.getNext();
      return parse_floating<f32, Lexeme::TKN_FLOAT_L>(lexer, snap);
    }
    if (lexer.next == 'd')
      lexer.next = lexer.getNext();
    parse_floating<f64, Lexeme::TKN_DOUBLE_L>(lexer, snap);
  }  
}