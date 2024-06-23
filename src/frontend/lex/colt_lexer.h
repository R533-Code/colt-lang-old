/*****************************************************************/ /**
 * @file   colt_lexer.h
 * @brief  Contains the lexing functions.
 *
 * @author RPC
 * @date   February 2024
 *********************************************************************/
#ifndef HG_COLT_LEXER
#define HG_COLT_LEXER

#include <array>
#include <exception>
#include "ascii.h"
#include "err/error_reporter.h"
#include "colt_token_buffer.h"

namespace clt::lng
{
  struct Lexer;

  /// @brief Lexes 'to_parse'
  /// @param reporter The reporter used to generate error/warnings/messages
  /// @param to_parse The StringView to parse
  /// @return A TokenBuffer containing parsed lexemes
  TokenBuffer lex(ErrorReporter& reporter, StringView to_parse) noexcept;

  /// @brief Lexes 'to_parse'.
  /// This does not clear 'buffer' first, use with caution!
  /// @param buffer The TokenBuffer in which to store lexing result
  /// @param reporter The reporter used to generate error/warnings/messages
  /// @param to_parse The StringView to parse
  /// @return A TokenBuffer containing parsed lexemes
  void lex(
      TokenBuffer& buffer, ErrorReporter& reporter, StringView to_parse) noexcept;

  /**
	* Functions starting with 'Consume' do not add any Token
	* to the token buffer.
	* Functions starting with 'Parse' add a Token to the
	* token buffer.
	*/

  /// @brief Consumes all characters till a whitespace is hit
  /// @param lexer The lexer used for parsing
  void consume_till_whitespaces(Lexer& lexer) noexcept;

  /// @brief Consumes all characters till a whitespace or a punctuation is hit
  /// @param lexer The lexer used for parsing
  void consume_till_space_or_punct(Lexer& lexer) noexcept;

  /// @brief Consumes all whitespaces till a non-whitespace is hit
  /// @param lexer The lexer used for parsing
  void consume_whitespaces(Lexer& lexer) noexcept;

  /// @brief Consumes all multi-line comments (recursive)
  /// @param lexer The lexer used for parsing
  /// @pre The '/*' of the comment must be consumed
  void consume_lines_comment(Lexer& lexer) noexcept;

  /// @brief Consumes all the digits (saving them in `lexer.temp`).
  /// This function does not clear `temp`.
  /// @param lexer The lexer used for parsing
  void consume_digits(Lexer& lexer) noexcept;

  /// @brief Consumes all the digits (with base 'base'), saving them in `lexer.temp`.
  /// This function does not clear `temp`.
  /// @param lexer The lexer used for parsing
  void consume_digits(Lexer& lexer, int base) noexcept;

  /// @brief Consumes all the alpha-numeric characters (saving them in `lexer.temp`).
  /// This function does not clear `temp`.
  /// @param lexer The lexer used for parsing
  void consume_alnum(Lexer& lexer) noexcept;

  /// @brief Parses an invalid character (consuming till a whitespace is hit)
  /// @param lexer The lexer used for parsing
  void parse_invalid(Lexer& lexer) noexcept;

  /// @brief Parses a '+'
  /// @param lexer The lexer used for parsing
  void parse_plus(Lexer& lexer) noexcept;

  /// @brief Parses a '-'
  /// @param lexer The lexer used for parsing
  void parse_minus(Lexer& lexer) noexcept;

  /// @brief Parses a '*'
  /// @param lexer The lexer used for parsing
  void parse_star(Lexer& lexer) noexcept;

  /// @brief Parses a '/'
  /// @param lexer The lexer used for parsing
  void parse_slash(Lexer& lexer) noexcept;

  /// @brief Parses a '%'
  /// @param lexer The lexer used for parsing
  void parse_percent(Lexer& lexer) noexcept;

  /// @brief Parses a ':'
  /// @param lexer The lexer used for parsing
  void parse_colon(Lexer& lexer) noexcept;

  /// @brief Parses a '='
  /// @param lexer The lexer used for parsing
  void parse_equal(Lexer& lexer) noexcept;

  /// @brief Parses a '!'
  /// @param lexer The lexer used for parsing
  void parse_exclam(Lexer& lexer) noexcept;

  /// @brief Parses a '<'
  /// @param lexer The lexer used for parsing
  void parse_lt(Lexer& lexer) noexcept;

  /// @brief Parses a '>'
  /// @param lexer The lexer used for parsing
  void parse_gt(Lexer& lexer) noexcept;

  /// @brief Parses a '&'
  /// @param lexer The lexer used for parsing
  void parse_and(Lexer& lexer) noexcept;

  /// @brief Parses a '|'
  /// @param lexer The lexer used for parsing
  void parse_or(Lexer& lexer) noexcept;

  /// @brief Parses a '^'
  /// @param lexer The lexer used for parsing
  void parse_caret(Lexer& lexer) noexcept;

  /// @brief Parses digits (floats or integrals)
  /// @param lexer The lexer used for parsing
  void parse_digit(Lexer& lexer) noexcept;

  /// @brief Parses an identifier
  /// @param lexer The lexer used for parsing
  void parse_identifier(Lexer& lexer) noexcept;

  template<Lexeme lexeme>
  /// @brief Parses a single character, adding 'lexeme' to the token buffer.
  /// @tparam lexeme The lexeme to add to the token buffer
  /// @param lexer The lexer used for parsing
  void parse_single(Lexer& lexer) noexcept;

  /// @brief Parses a '\.'
  /// @param lexer The lexer used for parsing
  void parse_dot(Lexer& lexer) noexcept;

  /// @brief Prints a token (used for debugging purposes)
  /// @param tkn The token to print
  /// @param buffer The token buffer (owns 'tkn')
  void print_token(Token tkn, const TokenBuffer& buffer) noexcept;

  template<typename T>
  /// @brief Returns the literal lexeme representing the C++ type 'T'
  /// @tparam T The type to convert to its lexeme equivalent
  /// @return The lexeme corresponding to 'T'
  constexpr Lexeme LiteralFromType() noexcept;

  /// @brief The type of the lexing functions callbacks
  using LexerDispatch_t = void (*)(Lexer&) noexcept;

  /// @brief Table containing lexing functions used for dispatch
  struct LexerDispatchTable : public std::array<LexerDispatch_t, 256>
  {
  };

  /// @brief Creates the table of lexing functions.
  /// The next character to parse is the index into the table.
  /// @return The table of lexing functions
  consteval LexerDispatchTable lexer_dispatch_table() noexcept
  {
    using enum clt::lng::Lexeme;
    LexerDispatchTable table{};
    for (size_t i = 0; i < table.size(); i++)
    {
      if (clt::isspace((char)i))
        table[i] = &consume_whitespaces;
      else if (clt::isdigit((char)i))
        table[i] = &parse_digit;
      else if (clt::isalpha((char)i))
        table[i] = &parse_identifier;
      else
        table[i] = &parse_invalid;
    }

    table['_'] = &parse_identifier;
    table['+'] = &parse_plus;
    table['-'] = &parse_minus;
    table['*'] = &parse_star;
    table['/'] = &parse_slash;
    table['%'] = &parse_percent;
    table[':'] = &parse_colon;
    table['='] = &parse_equal;
    table['!'] = &parse_exclam;
    table['.'] = &parse_dot;
    table['<'] = &parse_lt;
    table['>'] = &parse_gt;
    table['&'] = &parse_and;
    table['|'] = &parse_or;
    table['^'] = &parse_caret;
    table['~'] = &parse_single<TKN_TILDE>;
    table[';'] = &parse_single<TKN_SEMICOLON>;
    table[','] = &parse_single<TKN_COLON>;
    table['{'] = &parse_single<TKN_LEFT_CURLY>;
    table['}'] = &parse_single<TKN_RIGHT_CURLY>;
    table['('] = &parse_single<TKN_LEFT_PAREN>;
    table[')'] = &parse_single<TKN_RIGHT_PAREN>;
    table['['] = &parse_single<TKN_LEFT_SQUARE>;
    table[']'] = &parse_single<TKN_RIGHT_SQUARE>;

    return table;
  }

  struct Lexer
  {
    /// @brief The error reporter
    ErrorReporter& reporter;
    /// @brief TokenBuffer where to save the lexemes
    TokenBuffer& buffer;
    /// @brief The current line
    u32 _line_nb = 0;
    /// @brief The current offset into the current line
    u32 _offset = 0;
    /// @brief Temporary string used for literals
    String temp = {};
    /// @brief The size of the current lexeme
    u32 size_lexeme = 0;
    /// @brief Recursion depth for parsing comments
    u8 comment_depth = 0;
    /// @brief Next character to parse
    char _next;
#ifdef COLT_DEBUG
    bool ended = false;
#endif // COLT_DEBUG

    struct Snapshot
    {
      u32 line_nb;
      u32 column_nb;
    };

    /// @brief Size of the beginning of a multiline comment (SLASH STAR)
    static constexpr u32 MultilineCommentSize = 2;
    /// @brief The lexing table used to dispatch
    static constexpr LexerDispatchTable LexingTable = lexer_dispatch_table();

    template<typename... Args>
    StringView fmt(clt::io::fmt_str<Args...> fmt, Args&&... args) noexcept
    {
      return reporter.fmt(fmt, std::forward<Args>(args)...);
    }

    constexpr char next() noexcept
    {
      if (_line_nb == buffer.lines.size())
      {
#ifdef COLT_DEBUG
        assert_true("Do not call getNext() if EOF was already hit!", !ended);
        ended = true;
#endif // COLT_DEBUG
        return EOF;
      }
      ++size_lexeme;
      if (_offset == buffer.lines[_line_nb].size())
      {
        _offset = 0;
        ++_line_nb;
        return next();
      }
      return buffer.lines[_line_nb][_offset++];
    }

    constexpr u32 offset() const noexcept
    {
      assert_true(
          "getOffset can only be called after a call to getNext!",
          !(_offset == 0 && _line_nb == 0));

      return _offset ? _offset - 1
                     : static_cast<u32>(buffer.lines[_line_nb - 1].size());
    }

    constexpr Snapshot start_lexeme() noexcept
    {
      assert_true(
          "startLexeme can only be called after a call to getNext!",
          !(_offset == 0 && _line_nb == 0));

      size_lexeme = 0;
      return Snapshot{_line_nb, offset()};
    }

    constexpr StringView current_identifier(const Snapshot& snap) noexcept
    {
      return StringView{
          buffer.lines[snap.line_nb].data() + snap.column_nb,
          offset() - snap.column_nb};
    }

    /// @brief Look ahead in the string to scan
    /// @param offset The offset to add (0 being look ahead 1 character)
    /// @return The character 'offset + 1' after the current one
    constexpr char peek_next(u32 offset = 0) const noexcept
    {
      auto i = (i64)(this->_offset + offset) - (i64)buffer.lines[_line_nb].size();
      if (i < 0)
        return buffer.lines[_line_nb][this->_offset + offset];
      const auto next_line = _line_nb + 1;
      if (next_line == buffer.lines.size())
        return EOF;
      return buffer.lines[next_line][i];
    }

    /// @brief Creates a SourceInfo over a single-line lexeme
    /// @param start The start offset of the lexeme
    /// @return SourceInfo over the lexeme
    constexpr SourceInfo make_source(const Snapshot& snap) const noexcept
    {
      return SourceInfo{
          snap.line_nb + 1,
          StringView{
              &buffer.lines[snap.line_nb].front() + snap.column_nb, size_lexeme},
          buffer.lines[snap.line_nb]};
    }

    /// @brief Creates a SourceInfo over a single-line lexeme
    /// @param start The start offset of the lexeme
    /// @return SourceInfo over the lexeme
    constexpr SourceInfo make_source(u32 line_nb, u32 start) const noexcept
    {
      return SourceInfo{
          line_nb + 1,
          StringView{&buffer.lines[line_nb].front() + start, size_lexeme},
          buffer.lines[line_nb]};
    }

    /// @brief Creates a SourceInfo over a single-line lexeme
    /// @param start The start offset of the lexeme
    /// @param end The end offset of the lexeme
    /// @return SourceInfo over the lexeme
    constexpr SourceInfo make_source(u32 line_nb, u32 start, u32 end) const noexcept
    {
      return SourceInfo{
          line_nb + 1,
          StringView{&buffer.lines[line_nb].front() + start, end - start},
          buffer.lines[line_nb]};
    }

    /// @brief Saves a Token in the TokenBuffer
    /// @param lexeme The lexeme of the Token
    /// @param column_nb The starting column_nb of the Token
    void add_token(Lexeme lexeme, const Snapshot& snap) const noexcept
    {
      assert_true("Invalid call to add_token", size_lexeme != 0);
      buffer.add_token(lexeme, snap.line_nb, snap.column_nb, size_lexeme);
    }

    void add_identifier(StringView identifier, const Snapshot& snap) const noexcept
    {
      assert_true("Invalid call to addToken", size_lexeme != 0);
      buffer.add_identifier(
          identifier, Lexeme::TKN_IDENTIFIER, snap.line_nb, snap.column_nb,
          size_lexeme);
    }

    template<typename T>
    void add_literal(Lexeme lexeme, T value, const Snapshot& snap) const noexcept
    {
      assert_true("Verify lexeme and literal type!", LiteralFromType<T>() == lexeme);
      QWORD_t literal{};
      literal.bit_assign(value);
      buffer.add_literal(literal, lexeme, snap.line_nb, snap.column_nb, size_lexeme);
    }
  };

  /// @brief Handles the floating point contained in 'lexer.temp', consuming
  /// 'f' or 'd' extension if necessary.
  /// @param lexer The lexer used for parsing
  /// @param snap The source code informations of the floating point
  void handle_float_with_extension(
      Lexer& lexer, const Lexer::Snapshot& snap) noexcept;

  template<bool UnsignedOnly = false>
  /// @brief Handles the integer contained in 'lexer.temp', consuming
  /// [ui](8|16|32|64) extension if necessary.
  /// @tparam UnsignedOnly True if only unsigned extensions are allowed
  /// @param lexer The lexer used for parsing
  /// @param snap The source code informations of the integer
  /// @param base The base of the integer to parse
  void handle_int_with_extension(
      Lexer& lexer, const Lexer::Snapshot& snap, int base = 10) noexcept;

  template<typename T>
  constexpr Lexeme LiteralFromType() noexcept;

  template<meta::FloatingPoint T>
  /// @brief Converts the floating point in 'lexer.temp' and reports errors.
  /// @tparam T The floating point type to parse (f32 or f64)
  /// @param lexer The lexer used for parsing
  /// @param snap The source code informations of the integer
  void parse_floating(Lexer& lexer, const Lexer::Snapshot& snap) noexcept;

  template<meta::Integral T>
  /// @brief Converts the integer in 'lexer.temp' and reports errors.
  /// @tparam T The integer type to parse
  /// @param lexer The lexer used for parsing
  /// @param snap The source code informations of the integer
  void parse_integral(
      Lexer& lexer, const Lexer::Snapshot& snap, int base = 10) noexcept;

  /**
	* Template function implementations
	*/

  template<Lexeme lexeme>
  void parse_single(Lexer& lexer) noexcept
  {
    auto snap   = lexer.start_lexeme();
    lexer._next = lexer.next();
    lexer.add_token(lexeme, snap);
  }

  template<meta::FloatingPoint T>
  void parse_floating(Lexer& lexer, const Lexer::Snapshot& snap) noexcept
  {
    T value;
    auto result = clt::parse(lexer.temp, value);
    if (result.code() == ParsingCode::GOOD)
      return lexer.add_literal(LiteralFromType<T>(), value, snap);

    lexer.add_token(Lexeme::TKN_ERROR, snap);
    lexer.reporter.error(
        lexer.fmt("Invalid '{}' literal!", clt::reflect<T>::str()),
        lexer.make_source(snap));
  }

  template<meta::Integral T>
  void parse_integral(Lexer& lexer, const Lexer::Snapshot& snap, int base) noexcept
  {
    T value = 0;
    auto [ptr, err] =
        std::from_chars(lexer.temp.begin(), lexer.temp.end(), value, base);
    if (ptr == lexer.temp.end() && err == std::errc{})
      return lexer.add_literal(LiteralFromType<T>(), value, snap);

    lexer.add_token(Lexeme::TKN_ERROR, snap);
    lexer.reporter.error(
        lexer.fmt("Invalid '{}' literal!", clt::reflect<T>::str()),
        lexer.make_source(snap));
  }

  namespace details
  {
    template<typename dtype, typename found>
    void handle_1nb_int_extension(
        Lexer& lexer, const Lexer::Snapshot& snap, int base) noexcept
    {
      if (clt::isdigit(lexer.peek_next(1)))
        return parse_integral<dtype>(lexer, snap, base);
      // consume [iu]8
      lexer.next();
      lexer._next = lexer.next();
      return parse_integral<found>(lexer, snap, base);
    }

    template<typename dtype, typename found>
    void handle_2nb_int_extension(
        char chr, Lexer& lexer, const Lexer::Snapshot& snap, int base) noexcept
    {
      if (lexer.peek_next(1) != chr || clt::isdigit(lexer.peek_next(2)))
        return parse_integral<dtype>(lexer, snap, base);
      // consume [iu](16|32|64)
      lexer.next();
      lexer.next();
      lexer._next = lexer.next();
      return parse_integral<found>(lexer, snap, base);
    }
  } // namespace details

  template<bool UnsignedOnly>
  void handle_int_with_extension(
      Lexer& lexer, const Lexer::Snapshot& snap, int base) noexcept
  {
    // The default type to parse if no literal extension was found
    using dtype = std::conditional_t<UnsignedOnly, u64, i64>;

    switch (clt::tolower(lexer._next))
    {
      break;
    case 'u':
      switch (lexer.peek_next())
      {
      case '8':
        return details::handle_1nb_int_extension<dtype, u8>(lexer, snap, base);
      case '1':
        return details::handle_2nb_int_extension<dtype, u16>('6', lexer, snap, base);
      case '3':
        return details::handle_2nb_int_extension<dtype, u32>('2', lexer, snap, base);
      case '6':
        return details::handle_2nb_int_extension<dtype, u64>('4', lexer, snap, base);
      }
      // We make use of fallthrough if unsigned only
      break;
    case 'i':
      if constexpr (!UnsignedOnly)
      {
        switch (lexer.peek_next())
        {
        case '8':
          return details::handle_1nb_int_extension<dtype, i8>(lexer, snap, base);
        case '1':
          return details::handle_2nb_int_extension<dtype, i16>(
              '6', lexer, snap, base);
        case '3':
          return details::handle_2nb_int_extension<dtype, i32>(
              '2', lexer, snap, base);
        case '6':
          return details::handle_2nb_int_extension<dtype, i64>(
              '4', lexer, snap, base);
        }
        break;
      }
    default:
      return parse_integral<dtype>(lexer, snap, base);
    }
  }

  template<typename T>
  constexpr Lexeme LiteralFromType() noexcept
  {
    using enum Lexeme;

    if constexpr (std::same_as<T, bool>)
      return TKN_BOOL_L;
    if constexpr (std::same_as<T, char>)
      return TKN_CHAR_L;
    if constexpr (std::same_as<T, u8>)
      return TKN_U8_L;
    if constexpr (std::same_as<T, u16>)
      return TKN_U16_L;
    if constexpr (std::same_as<T, u32>)
      return TKN_U32_L;
    if constexpr (std::same_as<T, u64>)
      return TKN_U64_L;
    if constexpr (std::same_as<T, i8>)
      return TKN_I8_L;
    if constexpr (std::same_as<T, i16>)
      return TKN_I16_L;
    if constexpr (std::same_as<T, i32>)
      return TKN_I32_L;
    if constexpr (std::same_as<T, i64>)
      return TKN_I64_L;
    if constexpr (std::same_as<T, f32>)
      return TKN_FLOAT_L;
    if constexpr (std::same_as<T, f64>)
      return TKN_DOUBLE_L;
  }
} // namespace clt::lng

#endif // !HG_COLT_LEXER
