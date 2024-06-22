#include "colt_lexer.h"

namespace clt::lng
{
  /// @brief Consumes multi-line comments recursively.
  /// This function can throw an ExitRecursionExcept.
  /// @param lexer The lexer used to parse
  static void consume_lines_comment_throw(Lexer& lexer);

  void consume_till_whitespaces(Lexer& lexer) noexcept
  {
    // Consume till a whitespace or EOF is hit
    while (!clt::isspace(lexer._next) && lexer._next != EOF)
      lexer._next = lexer.next();
  }

  void consume_till_space_or_punct(Lexer& lexer) noexcept
  {
    // Consume till a whitespace or a punctuation or EOF is hit
    while (!clt::isspace(lexer._next) && !clt::ispunct(lexer._next) && lexer._next != EOF)
      lexer._next = lexer.next();
  }

  void consume_whitespaces(Lexer& lexer) noexcept
  {
    // Consume while whitespace and not EOF is hit
    while (clt::isspace(lexer._next) && lexer._next != EOF)
      lexer._next = lexer.next();
  }

  static void consume_lines_comment_throw(Lexer& lexer)
  {
    const u32 line_nb = lexer._line_nb;

    if (lexer.comment_depth == std::numeric_limits<u8>::max())
    {
      lexer.reporter.error("Exceeded recursion depth!");
      lexer._next = EOF; // To stop parsing
      throw ExitRecursionExcept{};
    }
    lexer.comment_depth++;
    assert_true("Invalid call to ConsumeLinesComment!", lexer._offset >= Lexer::MultilineCommentSize);
    const auto start_offset = lexer._offset - Lexer::MultilineCommentSize - 1;

    do
    {
      // We hit a nested comment
      if (lexer._next == '/' && lexer.peek_next() == '*')
      {
        lexer.next(); // consume '/'
        lexer._next = lexer.next(); // consume '*'
        consume_lines_comment_throw(lexer);
        continue;
      }
      if (lexer._next == '*' && lexer.peek_next() == '/')
      {
        lexer.next(); // consume '/'
        lexer._next = lexer.next();
        lexer.comment_depth--;
        return;
      }
      lexer._next = lexer.next();
    } while (lexer._next != EOF);

    // We hit EOF
    lexer.reporter.error("Unterminated multi-line comment!",
      lexer.make_source(line_nb, start_offset, start_offset + Lexer::MultilineCommentSize));
    throw ExitRecursionExcept{};
  }

  void consume_lines_comment(Lexer& lexer) noexcept
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

  void consume_digits(Lexer& lexer) noexcept
  {
    // Consume while is digit and not EOF is hit
    while (clt::isdigit(lexer._next) && lexer._next != EOF)
    {
      lexer.temp.push_back(lexer._next);
      lexer._next = lexer.next();
    }
  }

  void consume_digits(Lexer& lexer, int base) noexcept
  {
    assert_true("Invalid base!", base > 1, base <= 16);
    if (base <= 10)
    {
      while (('0' <= lexer._next && lexer._next < '0' + base) && lexer._next != EOF)
      {
        lexer.temp.push_back(lexer._next);
        lexer._next = lexer.next();
      }
    }
    else
    {
      const int minus_10 = base - 10;
      while ((('0' <= lexer._next && lexer._next < '0' + base)
        || ('A' <= clt::toupper(lexer._next) &&  clt::toupper(lexer._next) < 'A' + minus_10))
        && lexer._next != EOF)
      {
        lexer.temp.push_back(lexer._next);
        lexer._next = lexer.next();
      }
    }
  }

  void consume_alnum(Lexer& lexer) noexcept
  {
    // Consume while is alnum and not EOF is hit
    while (clt::isalnum(lexer._next) && lexer._next != EOF)
    {
      lexer.temp.push_back(lexer._next);
      lexer._next = lexer.next();
    }
  }

  void parse_invalid(Lexer& lexer) noexcept
  {
    auto snap = lexer.start_lexeme();

    lexer._next = lexer.next();
    consume_till_space_or_punct(lexer);
    lexer.add_token(Lexeme::TKN_ERROR, snap);

    // TODO: add error number
    lexer.reporter.error("Invalid character!", lexer.make_source(snap));
  }

  void parse_plus(Lexer& lexer) noexcept
  {
    auto snap = lexer.start_lexeme();

    lexer._next = lexer.next();
    switch (lexer._next)
    {
    break; case '=':
      lexer._next = lexer.next(); // consume '='
      lexer.add_token(Lexeme::TKN_PLUS_EQUAL, snap);
    break; case '+':
      lexer._next = lexer.next(); // consume '+'
      lexer.add_token(Lexeme::TKN_PLUS_PLUS, snap);
    break; default:
      lexer.add_token(Lexeme::TKN_PLUS, snap);
    }
  }

  void parse_minus(Lexer& lexer) noexcept
  {
    auto snap = lexer.start_lexeme();

    lexer._next = lexer.next();
    switch (lexer._next)
    {
    break; case '=':
      lexer._next = lexer.next(); // consume '='
      lexer.add_token(Lexeme::TKN_MINUS_EQUAL, snap);
    break; case '-':
      lexer._next = lexer.next(); // consume '-'
      lexer.add_token(Lexeme::TKN_MINUS_MINUS, snap);
    break; default:
      lexer.add_token(Lexeme::TKN_MINUS, snap);
    }
  }

  void parse_star(Lexer& lexer) noexcept
  {
    auto snap = lexer.start_lexeme();

    lexer._next = lexer.next();
    if (lexer._next == '=')
    {
      lexer._next = lexer.next(); // consume '='
      lexer.add_token(Lexeme::TKN_STAR_EQUAL, snap);
    }
    else
      lexer.add_token(Lexeme::TKN_STAR, snap);
  }

  void parse_slash(Lexer& lexer) noexcept
  {
    auto snap = lexer.start_lexeme();

    lexer._next = lexer.next();
    switch (lexer._next)
    {
    break; case '=':
      lexer._next = lexer.next(); // consume '='
      lexer.add_token(Lexeme::TKN_SLASH_EQUAL, snap);

      /****  COMMENTS HANDLING  ****/
    break; case '/':
      // Go to next line
      lexer._line_nb++;
      lexer._offset = 0;
    break; case '*':
      lexer._next = lexer.next(); // consume '*'
      consume_lines_comment(lexer);
    break; default:
      lexer.add_token(Lexeme::TKN_SLASH, snap);
    }
  }

  void parse_percent(Lexer& lexer) noexcept
  {
    auto snap = lexer.start_lexeme();

    lexer._next = lexer.next();
    if (lexer._next == '=')
    {
      lexer._next = lexer.next(); // consume '='
      lexer.add_token(Lexeme::TKN_PERCENT_EQUAL, snap);
    }
    else
      lexer.add_token(Lexeme::TKN_PERCENT, snap);
  }

  void parse_colon(Lexer& lexer) noexcept
  {
    auto snap = lexer.start_lexeme();

    lexer._next = lexer.next();
    if (lexer._next == ':')
    {
      lexer._next = lexer.next(); // consume ':'
      lexer.add_token(Lexeme::TKN_COLON_COLON, snap);
    }
    else
      lexer.add_token(Lexeme::TKN_COLON, snap);
  }

  void parse_equal(Lexer& lexer) noexcept
  {
    auto snap = lexer.start_lexeme();

    lexer._next = lexer.next();
    switch (lexer._next)
    {
    break; case '=':
      lexer._next = lexer.next(); // consume '='
      lexer.add_token(Lexeme::TKN_EQUAL_EQUAL, snap);
    break; case '>':
      lexer._next = lexer.next(); // consume '>'
      lexer.add_token(Lexeme::TKN_EQUAL_GREAT, snap);
    break; default:
      lexer.add_token(Lexeme::TKN_EQUAL, snap);
    }
  }

  void parse_exclam(Lexer& lexer) noexcept
  {
    auto snap = lexer.start_lexeme();

    lexer._next = lexer.next();
    if (lexer._next == '=')
    {
      lexer._next = lexer.next(); // consume '='
      lexer.add_token(Lexeme::TKN_EXCLAM_EQUAL, snap);
    }
    else
      lexer.add_token(Lexeme::TKN_EXCLAM, snap);
  }

  void parse_lt(Lexer& lexer) noexcept
  {
    auto snap = lexer.start_lexeme();

    lexer._next = lexer.next();
    switch (lexer._next)
    {
    break; case '=':
      lexer._next = lexer.next(); // consume '='
      lexer.add_token(Lexeme::TKN_LESS_EQUAL, snap);

    break; case '<':
      lexer._next = lexer.next(); // consume '<'
      if (lexer._next == '=')
      {
        lexer._next = lexer.next(); // consume '='
        lexer.add_token(Lexeme::TKN_LESS_LESS_EQUAL, snap);
      }
      else
        lexer.add_token(Lexeme::TKN_LESS_LESS, snap);

    break; default:
      lexer.add_token(Lexeme::TKN_LESS, snap);
    }
  }

  void parse_gt(Lexer& lexer) noexcept
  {
    auto snap = lexer.start_lexeme();

    lexer._next = lexer.next();
    switch (lexer._next)
    {
    break; case '=':
      lexer._next = lexer.next(); // consume '='
      lexer.add_token(Lexeme::TKN_GREAT_EQUAL, snap);

    break; case '>':
      lexer._next = lexer.next(); // consume '>'
      if (lexer._next == '=')
      {
        lexer._next = lexer.next(); // consume '='
        lexer.add_token(Lexeme::TKN_GREAT_GREAT_EQUAL, snap);
      }
      else
        lexer.add_token(Lexeme::TKN_GREAT_GREAT, snap);

    break; default:
      lexer.add_token(Lexeme::TKN_GREAT, snap);
    }
  }

  void parse_and(Lexer& lexer) noexcept
  {
    auto snap = lexer.start_lexeme();

    lexer._next = lexer.next();
    switch (lexer._next)
    {
    break; case '=':
      lexer._next = lexer.next(); // consume '='
      lexer.add_token(Lexeme::TKN_AND_EQUAL, snap);
    break; case '&':
      lexer._next = lexer.next(); // consume '&'
      lexer.add_token(Lexeme::TKN_AND_AND, snap);
    break; default:
      lexer.add_token(Lexeme::TKN_AND, snap);
    }
  }

  void parse_or(Lexer& lexer) noexcept
  {
    auto snap = lexer.start_lexeme();

    lexer._next = lexer.next();
    switch (lexer._next)
    {
    break; case '=':
      lexer._next = lexer.next(); // consume '='
      lexer.add_token(Lexeme::TKN_OR_EQUAL, snap);
    break; case '|':
      lexer._next = lexer.next(); // consume '|'
      lexer.add_token(Lexeme::TKN_OR_OR, snap);
    break; default:
      lexer.add_token(Lexeme::TKN_OR, snap);
    }
  }

  void parse_caret(Lexer& lexer) noexcept
  {
    auto snap = lexer.start_lexeme();

    lexer._next = lexer.next();
    if (lexer._next == '=')
    {
      lexer._next = lexer.next(); // consume '='
      lexer.add_token(Lexeme::TKN_CARET_EQUAL, snap);
    }
    else
      lexer.add_token(Lexeme::TKN_CARET, snap);
  }

  void parse_digit(Lexer& lexer) noexcept
  {
    auto snap = lexer.start_lexeme();
    lexer.temp.clear();
    lexer.temp.push_back(lexer._next);
    
    if (lexer._next == '0') //Could be 0x, 0b, 0o
    {
      lexer._next = lexer.next();
      char symbol = lexer._next;
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
          return handle_int_with_extension(lexer, snap);
      }
      lexer._next = lexer.next(); //Consume symbol
      //Pop the leading '0'
      lexer.temp.clear();
      consume_digits(lexer, base);

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
        consume_till_space_or_punct(lexer);
        lexer.reporter.error(range_str, lexer.make_source(snap));
        return lexer.add_token(Lexeme::TKN_ERROR, snap);
      }
      return handle_int_with_extension<true>(lexer, snap, base);
    }
    lexer._next = lexer.next();
  NORM:
    //Parse as many digits as possible
    consume_digits(lexer);

    bool is_float = false;
    // [0-9]+ followed by a .[0-9] is a float
    if (lexer._next == '.')
    {
      //Snapshot for '.' character
      // TODO: fix snapshot...
      //auto snap_dot = lexer.start_lexeme();

      lexer._next = lexer.next();
      if (clt::isdigit(lexer._next))
      {
        is_float = true;
        lexer.temp.push_back('.');
        lexer.temp.push_back(lexer._next);
        lexer._next = lexer.next();

        //Parse as many digits as possible
        consume_digits(lexer);
      }
      else
      {
        //We parse the integer
        parse_integral<i64>(lexer, snap);
                
        //The dot is not followed by a digit, this is not a float,
        //but rather should be a dot followed by an identifier for a function call
        lexer._next = lexer.next();
        return lexer.add_token(Lexeme::TKN_DOT, snap);
      }
    }

    // [0-9]+(.[0-9]+)?e[+-][0-9]+ is a float
    // We are possibly parsing an exponent
    if (lexer._next == 'e')
    {
      char after_e = lexer.peek_next();
      if (clt::isdigit(after_e))
      {
        is_float = true;
        lexer._next = lexer.next(); // consume 'e'
        lexer.temp.push_back('e');
        consume_digits(lexer);
      }
      else if (after_e == '+' && clt::isdigit(lexer.peek_next(1)))
      {
        is_float = true;
        lexer.next(); // consume 'e'
        lexer._next = lexer.next(); // consume '+'
        lexer.temp.push_back('e');
        consume_digits(lexer);
      }
      else if (after_e == '-' && clt::isdigit(lexer.peek_next(1)))
      {
        is_float = true;
        lexer.next(); // consume 'e'
        lexer._next = lexer.next(); // consume '-'
        lexer.temp.push_back("e-");
        consume_digits(lexer);
      }
    }

    if (is_float)
      handle_float_with_extension(lexer, snap);
    else
      handle_int_with_extension(lexer, snap);
  }

  /// @brief Map from keyword string to lexeme
  static constexpr auto KeywordMap = keyword_map();

  void parse_identifier(Lexer& lexer) noexcept
  {
    auto snap = lexer.start_lexeme();

    // Consume till a whitespace or EOF is hit
    while (clt::isalnum(lexer._next) || lexer._next == '_')
      lexer._next = lexer.next();
    
    StringView identifier = lexer.current_identifier(snap);
    // This is a keyword
    if (auto keyword_opt = KeywordMap.find(identifier); keyword_opt.is_value())
      return lexer.add_token(*keyword_opt, snap);

    if (identifier.starts_with("___"))
    {
      lexer.reporter.error(
        "Identifiers starting with '___' are reserved for the compiler!",
        lexer.make_source(snap));
      // TODO: add identifier but increment error count
      return lexer.add_token(Lexeme::TKN_ERROR, snap);
    }
    lexer.add_identifier(identifier, snap);
  }

  void parse_dot(Lexer& lexer) noexcept
  {
    auto snap = lexer.start_lexeme();

    lexer._next = lexer.next();
    if (!clt::isdigit(lexer._next))
      return lexer.add_token(Lexeme::TKN_DOT, snap);

    //Clear the string
    lexer.temp.clear();
    lexer.temp.push_back('.');
    consume_digits(lexer);

    // We are possibly parsing an exponent
    if (lexer._next == 'e')
    {
      char after_e = lexer.peek_next();
      if (clt::isdigit(after_e))
      {
        lexer._next = lexer.next(); // consume 'e'
        lexer.temp.push_back('e');
        consume_digits(lexer);
      }
      else if (after_e == '+' && clt::isdigit(lexer.peek_next(1)))
      {
        lexer.next(); // consume 'e'
        lexer._next = lexer.next(); // consume '+'
        lexer.temp.push_back('e');
        consume_digits(lexer);
      }
      else if (after_e == '-' && clt::isdigit(lexer.peek_next(1)))
      {
        lexer.next(); // consume 'e'
        lexer._next = lexer.next(); // consume '-'
        lexer.temp.push_back("e-");
        consume_digits(lexer);
      }
    }
    handle_float_with_extension(lexer, snap);
  }

  void print_token(Token tkn, const TokenBuffer& buffer) noexcept
  {
    using enum Lexeme;
    
    if (is_literal(tkn) && tkn != TKN_STRING_L)
    {
      switch_no_default(tkn.lexeme())
      {
      case TKN_BOOL_L:
        return io::print("{:h} {}", tkn.lexeme(), buffer.literal(tkn).as<bool>());
      case TKN_CHAR_L:
        return io::print("{:h} {}", tkn.lexeme(), buffer.literal(tkn).as<char>());      
      case TKN_U8_L:
      case TKN_U16_L:
      case TKN_U32_L:
      case TKN_U64_L:
        return io::print("{:h} {}", tkn.lexeme(), buffer.literal(tkn).as<u64>());      
      case TKN_I8_L:
        return io::print("{:h} {}", tkn.lexeme(), buffer.literal(tkn).as<i8>());      
      case TKN_I16_L:
        return io::print("{:h} {}", tkn.lexeme(), buffer.literal(tkn).as<i16>());      
      case TKN_I32_L:
        return io::print("{:h} {}", tkn.lexeme(), buffer.literal(tkn).as<i32>());      
      case TKN_I64_L:
        return io::print("{:h} {}", tkn.lexeme(), buffer.literal(tkn).as<i64>());
      case TKN_FLOAT_L:
        return io::print("{:h} {}", tkn.lexeme(), buffer.literal(tkn).as<f32>());      
      case TKN_DOUBLE_L:
        return io::print("{:h} {}", tkn.lexeme(), buffer.literal(tkn).as<f64>());
      }
    }
    else if (tkn == TKN_IDENTIFIER)
      return io::print("{:h} {}", tkn.lexeme(), buffer.identifier(tkn));
    return io::print("{:h}", tkn.lexeme());
  }
  
  void handle_float_with_extension(Lexer& lexer, const Lexer::Snapshot& snap) noexcept
  {
    if (lexer._next == 'f')
    {
      lexer._next = lexer.next();
      return parse_floating<f32>(lexer, snap);
    }
    if (lexer._next == 'd')
      lexer._next = lexer.next();
    parse_floating<f64>(lexer, snap);
  }  
}