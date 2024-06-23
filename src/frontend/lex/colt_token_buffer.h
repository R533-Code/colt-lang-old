/*****************************************************************/ /**
 * @file   colt_token_buffer.h
 * @brief  Contains the TokenBuffer helpers.
 * The result of lexing is a TokenBuffer: its design follows data
 * oriented principles. See the C++ talk about modernizing compiler
 * design.
 *
 * @author RPC
 * @date   February 2024
 *********************************************************************/
#ifndef HG_COLT_TOKEN_BUFFER
#define HG_COLT_TOKEN_BUFFER

#include "structs/list.h"
#include "structs/set.h"
#include "colt_operators.h"
#include "err/error_reporter.h"

namespace clt::lng
{
  // Forward declaration
  class TokenBuffer;
  // Forward declaration
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

  /// @brief Breaks down a StringView into lines
  /// @param strv The StringView to break down into lines
  /// @param buffer The buffer where to append these lines
  void create_lines(StringView strv, TokenBuffer& buffer) noexcept;

  /// @brief Contains information about a Token
  struct TokenInfo
  {
    /// @brief 0-based column_nb
    u32 column_nb;
    /// @brief Size of the lexeme
    u32 size;
    /// @brief 0-based start line number
    u32 line_start;
    /// @brief 0-based end line number
    u32 line_end;

    /// @brief Check if the Token spans on a single line
    /// @return True if the Token spans on a single line
    constexpr bool is_single_line() const noexcept { return line_end == line_start; }
  };

  class Token
  {
    /// @brief The actual lexeme
    u32 _lexeme : 8;
    /// @brief 0-based index into the array of literals.
    /// The array in which to index depends on the lexeme.
    u32 literal_index : 24;
    /// @brief Index into the array of information of each lexeme
    u32 info_index;

#ifdef COLT_DEBUG
    /// @brief On debug, we store the ID of the TokenBuffer owning the Token
    u32 buffer_id;

    constexpr Token(
        Lexeme lexeme, u32 info_index, u32 buffer_id, u32 literal = 0) noexcept
        : _lexeme(static_cast<u8>(lexeme))
        , literal_index(literal)
        , info_index(info_index)
        , buffer_id(buffer_id)
    {
    }

#else
    constexpr Token(Lexeme lexeme, u32 info_index, u32 literal = 0) noexcept
        : _lexeme(static_cast<u8>(lexeme))
        , literal_index(literal)
        , info_index(info_index)
    {
    }
#endif // COLT_DEBUG

  public:
    friend class TokenBuffer;

    Token()                                           = delete;
    constexpr Token(Token&&) noexcept                 = default;
    constexpr Token(const Token&) noexcept            = default;
    constexpr Token& operator=(Token&&) noexcept      = default;
    constexpr Token& operator=(const Token&) noexcept = default;

    /// @brief Converts a Token to the Lexeme it represents
    constexpr operator Lexeme() const noexcept
    {
      return static_cast<Lexeme>(_lexeme);
    }

    /// @brief Returns the Lexeme the Token represents
    /// @return The Lexeme representing the Token
    constexpr Lexeme lexeme() const noexcept { return static_cast<Lexeme>(_lexeme); }
  };

  /// @brief Represents a range of Token
  class TokenRange
  {
    /// @brief The beginning of the range
    u32 start_index;
    /// @brief The end of the range (non-inclusive)
    u32 end_index;

#ifdef COLT_DEBUG
    /// @brief On debug, we store the ID of the TokenBuffer owning the Token
    u32 buffer_id;

    /// @brief Constructs a range of Token
    /// @param start The start of the range
    /// @param end The end of the range
    /// @param buffer_id The buffer ID (used on Debug)
    constexpr TokenRange(u32 start, u32 end, u32 buffer_id) noexcept
        : start_index(start)
        , end_index(end)
        , buffer_id(buffer_id)
    {
    }

#else
    /// @brief Constructs a range of Token
    /// @param start The start of the range
    /// @param end The end of the range
    constexpr TokenRange(u32 start, u32 end) noexcept
        : start_index(start)
        , end_index(end)
    {
    }
#endif // COLT_DEBUG
    friend class TokenBuffer;

  public:
    TokenRange()                                                = delete;
    constexpr TokenRange(TokenRange&&) noexcept                 = default;
    constexpr TokenRange(const TokenRange&) noexcept            = default;
    constexpr TokenRange& operator=(TokenRange&&) noexcept      = default;
    constexpr TokenRange& operator=(const TokenRange&) noexcept = default;
  };

  class TokenBuffer
  {
    /// @brief The array of identifiers
    StableSet<StringView> identifiers{};
    /// @brief The array of lines
    FlatList<StringView, 256> lines{};
    /// @brief The array of string literals
    FlatList<UniquePtr<String>, 256> str_literals{};
    /// @brief The array of literal numbers (including f32/f64)
    FlatList<QWORD_t, 256> nb_literals{};
    /// @brief The array of token information
    FlatList<TokenInfo, 512> tokens_info{};
    /// @brief The array of tokens
    FlatList<Token, 512> tokens{};

#ifdef COLT_DEBUG
    /// @brief Used to differentiate different TokenBuffer
    static std::atomic<u32> ID_GENERATOR;

    /// @brief The ID of the current buffer
    u32 buffer_id;
#endif // COLT_DEBUG

    /// @brief Check if a Token is owned by the current TokenBuffer
    constexpr void owns(Token tkn) const noexcept
    {
#ifdef COLT_DEBUG
      if constexpr (is_debug_build())
        assert_true(
            "Token is not owned by this TokenBuffer!", tkn.buffer_id == buffer_id);
#endif // COLT_DEBUG
    }

    /// @brief Check if a TokenRange is owned by the current TokenBuffer
    constexpr void owns(TokenRange tkn) const noexcept
    {
#ifdef COLT_DEBUG
      if constexpr (is_debug_build())
        assert_true(
            "Token is not owned by this TokenBuffer!", tkn.buffer_id == buffer_id);
#endif // COLT_DEBUG
    }

    // Friend declaration to use add_token
    friend struct Lexer;

  public:
    /// @brief Default constructor
    TokenBuffer() noexcept
    {
#ifdef COLT_DEBUG
      if constexpr (is_debug_build())
        buffer_id = ID_GENERATOR.fetch_add(1, std::memory_order_acq_rel);
#endif // COLT_DEBUG
    }

    TokenBuffer(const TokenBuffer&)            = delete;
    TokenBuffer& operator=(const TokenBuffer&) = delete;

    TokenBuffer(TokenBuffer&&) noexcept            = default;
    TokenBuffer& operator=(TokenBuffer&&) noexcept = delete;

    /// @brief Clears the TokenBuffer
    void unsafe_clear() noexcept
    {
      lines.clear();
      str_literals.clear();
      nb_literals.clear();
      tokens_info.clear();
      tokens.clear();
    }

    /// @brief Adds a line
    /// @param line The line to save
    void add_line(StringView line) noexcept { lines.push_back(line); }

    /// @brief Generates a Token
    /// @param lexeme The lexeme of the Token
    /// @param line The line number
    /// @param column_nb The column_nb of the line
    /// @param literal The literal index
    /// @return Generated Token
    void add_token(Lexeme lexeme, u32 line, u32 column_nb, u32 size) noexcept
    {
      assert_true(
          "Integer overflow!", tokens_info.size() < std::numeric_limits<u32>::max());
#ifdef COLT_DEBUG
      tokens.push_back(
          Token{lexeme, static_cast<u32>(tokens_info.size()), buffer_id});
      tokens_info.push_back(TokenInfo{column_nb, size, line, line});
#else
      tokens.push_back(Token{lexeme, static_cast<u32>(tokens_info.size())});
      tokens_info.push_back(TokenInfo{column_nb, size, line, line});
#endif // COLT_DEBUG
    }

    TokenRange range_from(Token start) const noexcept
    {
      assert_true("Invalid range!", start == Lexeme::TKN_EOF);
#ifdef COLT_DEBUG
      owns(start);
      return TokenRange{start.info_index, start.info_index + 1, buffer_id};
#else
      return TokenRange{start.info_index, start.info_index + 1};
#endif // COLT_DEBUG
    }

    TokenRange range_from(Token start, Token end) const noexcept
    {
      assert_true("Invalid range!", start.info_index <= end.info_index);
#ifdef COLT_DEBUG
      owns(start), owns(end);
      return TokenRange{start.info_index, end.info_index, buffer_id};
#else
      return TokenRange{start.info_index, end.info_index};
#endif // COLT_DEBUG
    }

    void add_identifier(
        StringView value, Lexeme lexeme, u32 line, u32 column_nb, u32 size) noexcept
    {
      u64 ret = identifiers.size();
      identifiers.push_back(value);
      assert_true("Integer overflow!", ret <= std::numeric_limits<u32>::max());

#ifdef COLT_DEBUG
      tokens.push_back(Token{
          lexeme, static_cast<u32>(tokens_info.size()), buffer_id,
          static_cast<u32>(ret)});
      tokens_info.push_back(TokenInfo{column_nb, size, line, line});
#else
      tokens.push_back(Token{
          lexeme, static_cast<u32>(tokens_info.size()), static_cast<u32>(ret)});
      tokens_info.push_back(TokenInfo{column_nb, size, line, line});
#endif // COLT_DEBUG
    }

    void add_literal(
        QWORD_t value, Lexeme lexeme, u32 line, u32 column_nb, u32 size) noexcept
    {
      u64 ret = nb_literals.size();
      nb_literals.push_back(value);
      assert_true("Integer overflow!", ret <= std::numeric_limits<u32>::max());

#ifdef COLT_DEBUG
      tokens.push_back(Token{
          lexeme, static_cast<u32>(tokens_info.size()), buffer_id,
          static_cast<u32>(ret)});
      tokens_info.push_back(TokenInfo{column_nb, size, line, line});
#else
      tokens.push_back(Token{
          lexeme, static_cast<u32>(tokens_info.size()), static_cast<u32>(ret)});
      tokens_info.push_back(TokenInfo{column_nb, size, line, line});
#endif // COLT_DEBUG
    }

    /// @brief Returns a StringView over the line in which the token appears
    /// @param tkn The Token whose line to return
    /// @return The line in which appears the token
    StringView line_str(Token tkn) const noexcept
    {
      owns(tkn);
      return lines[tokens_info[tkn.info_index].line_start];
    }

    /// @brief Returns the line number on which the token appears
    /// @param tkn The Token whose line to return
    /// @return The line in which appears the token (1-based)
    u32 line_nb(Token tkn) const noexcept
    {
      owns(tkn);
      return tokens_info[tkn.info_index].line_start + 1;
    }

    /// @brief Returns the column_nb on which the token appears
    /// @param tkn The Token whose column_nb to return
    /// @return The column_nb to return (1-based)
    u32 column_nb(Token tkn) const noexcept
    {
      owns(tkn);
      return tokens_info[tkn.info_index].column_nb + 1;
    }

    StringView identifier(Token tkn) const noexcept
    {
      assert_true(
          "Token does not represent an identifier!",
          tkn.lexeme() == Lexeme::TKN_IDENTIFIER);
      owns(tkn);
      return identifiers[tkn.literal_index];
    }

    QWORD_t literal(Token tkn) const noexcept
    {
      assert_true(
          "Token does not represent a literal!", is_literal(tkn.lexeme()),
          tkn.lexeme() != Lexeme::TKN_STRING_L);
      owns(tkn);
      return nb_literals[tkn.literal_index];
    }

    /// @brief Returns the size of the Token
    /// @param tkn The Token whose size to return
    /// @return The column_nb to return (1-based)
    TokenInfo info(Token tkn) const noexcept
    {
      owns(tkn);
      return tokens_info[tkn.info_index];
    }

    /// @brief Constructs a source information from a token range
    /// @param range The token range
    /// @return SourceInfo represented by the token range
    SourceInfo make_source_info(TokenRange range) const noexcept
    {
      owns(range);
      auto& tkn1_info = tokens_info[range.start_index];
      auto& tkn2_info = tokens_info[range.end_index - 1];
      auto line       = StringView{
          lines[tkn1_info.line_start].data(),
          lines[tkn2_info.line_end].data() + lines[tkn2_info.line_end].size()};
      auto expr = StringView{
          lines[tkn1_info.line_start].data() + tkn1_info.column_nb,
          lines[tkn2_info.line_end].data() + tkn2_info.size + tkn2_info.column_nb};
      return SourceInfo{
          tkn1_info.line_start + 1, tkn2_info.line_end + 1, expr, line};
    }

    /// @brief Constructs a source information from a token
    /// @param tkn The token
    /// @return SourceInfo represented by the token
    SourceInfo make_source_info(Token tkn) const noexcept
    {
      owns(tkn);
      auto& tkn_info = tokens_info[tkn.info_index];
      auto expr      = StringView{
          lines[tkn_info.line_start].data() + tkn_info.column_nb,
          lines[tkn_info.line_end].data() + tkn_info.size + tkn_info.column_nb};
      return SourceInfo{tkn_info.line_start + 1, expr, lines[tkn_info.line_start]};
    }

    /// @brief Returns the list of tokens
    /// @return List of tokens
    auto& token_buffer() const noexcept { return tokens; }

    /// @brief Returns the list of lines
    /// @return The list of lines
    auto& line_buffer() const noexcept { return lines; }
  };
} // namespace clt::lng

#endif // !HG_COLT_TOKEN_BUFFER