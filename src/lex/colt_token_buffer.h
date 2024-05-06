/*****************************************************************//**
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
  TokenBuffer Lex(ErrorReporter& reporter, StringView to_parse) noexcept;

  /// @brief Lexes 'to_parse'.
  /// This does not clear 'buffer' first, use with caution!
  /// @param buffer The TokenBuffer in which to store lexing result
  /// @param reporter The reporter used to generate error/warnings/messages
  /// @param to_parse The StringView to parse
  /// @return A TokenBuffer containing parsed lexemes
  void Lex(TokenBuffer& buffer, ErrorReporter& reporter, StringView to_parse) noexcept;

  /// @brief Breaks down a StringView into lines
  /// @param strv The StringView to break down into lines
  /// @param buffer The buffer where to append these lines
  void CreateLines(StringView strv, TokenBuffer& buffer) noexcept;

  /// @brief Contains information about a Token
  struct TokenInfo
  {
    /// @brief 0-based column
    u32 column;
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
    u32 lexeme: 8;
    /// @brief 0-based index into the array of literals.
    /// The array in which to index depends on the lexeme.
    u32 literal_index: 24;
    /// @brief Index into the array of information of each lexeme
    u32 info_index;

#ifdef COLT_DEBUG
    /// @brief On debug, we store the ID of the TokenBuffer owning the Token
    u32 buffer_id;

    constexpr Token(Lexeme lexeme, u32 info_index, u32 buffer_id, u32 literal = 0) noexcept
      : lexeme(static_cast<u8>(lexeme)), literal_index(literal), info_index(info_index), buffer_id(buffer_id) {}

#else
    constexpr Token(Lexeme lexeme, u32 info_index, u32 literal = 0) noexcept
      : lexeme(static_cast<u8>(lexeme)), literal_index(literal), info_index(info_index) {}
#endif // COLT_DEBUG


  public:
    friend class TokenBuffer;

    Token() = delete;
    constexpr Token(Token&&) noexcept = default;
    constexpr Token(const Token&) noexcept = default;
    constexpr Token& operator=(Token&&) noexcept = default;
    constexpr Token& operator=(const Token&) noexcept = default;

    /// @brief Converts a Token to the Lexeme it represents
    constexpr operator Lexeme() const noexcept { return static_cast<Lexeme>(lexeme); }

    /// @brief Returns the Lexeme the Token represents
    /// @return The Lexeme representing the Token
    constexpr Lexeme getLexeme() const noexcept { return static_cast<Lexeme>(lexeme); }
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
      : start_index(start), end_index(end), buffer_id(buffer_id) {}

#else
    /// @brief Constructs a range of Token
    /// @param start The start of the range
    /// @param end The end of the range
    constexpr TokenRange(u32 start, u32 end) noexcept
      : start_index(start), end_index(end) {}
#endif // COLT_DEBUG
    friend class TokenBuffer;
  
  public:
    TokenRange() = delete;
    constexpr TokenRange(TokenRange&&) noexcept = default;
    constexpr TokenRange(const TokenRange&) noexcept = default;
    constexpr TokenRange& operator=(TokenRange&&) noexcept = default;
    constexpr TokenRange& operator=(const TokenRange&) noexcept = default;
  };

  class TokenBuffer
  {
    /// @brief The array of identifiers
    StableSet<StringView>             identifiers{};
    /// @brief The array of lines
    FlatList<StringView, 256>         lines{};
    /// @brief The array of string literals
    FlatList<UniquePtr<String>, 256>  str_literals{};
    /// @brief The array of literal numbers (including f32/f64)
    FlatList<QWORD_t, 256>            nb_literals{};
    /// @brief The array of token information
    FlatList<TokenInfo, 512>          tokens_info{};
    /// @brief The array of tokens
    FlatList<Token, 512>              tokens{};

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
      if constexpr (isDebugBuild())
        assert_true("Token is not owned by this TokenBuffer!", tkn.buffer_id == buffer_id);
#endif // COLT_DEBUG
    }
    
    /// @brief Check if a TokenRange is owned by the current TokenBuffer
    constexpr void owns(TokenRange tkn) const noexcept
    {
#ifdef COLT_DEBUG
      if constexpr (isDebugBuild())
        assert_true("Token is not owned by this TokenBuffer!", tkn.buffer_id == buffer_id);
#endif // COLT_DEBUG
    }

    // Friend declaration to use addToken
    friend struct Lexer;

  public:
    /// @brief Default constructor
    TokenBuffer() noexcept
    {
#ifdef COLT_DEBUG
      if constexpr (isDebugBuild())
        buffer_id = ID_GENERATOR.fetch_add(1, std::memory_order_acq_rel);
#endif // COLT_DEBUG
    }

    TokenBuffer(const TokenBuffer&) = delete;
    TokenBuffer& operator=(const TokenBuffer&) = delete;

    TokenBuffer(TokenBuffer&&) noexcept = default;
    TokenBuffer& operator=(TokenBuffer&&) noexcept = delete;

    /// @brief Clears the TokenBuffer
    void unsafeClear() noexcept
    {
      lines.clear();
      str_literals.clear();
      nb_literals.clear();
      tokens_info.clear();
      tokens.clear();
    }

    /// @brief Adds a line
    /// @param line The line to save
    void addLine(StringView line) noexcept
    {
      lines.push_back(line);
    }

    /// @brief Generates a Token
    /// @param lexeme The lexeme of the Token
    /// @param line The line number
    /// @param column The column of the line
    /// @param literal The literal index
    /// @return Generated Token
    void addToken(Lexeme lexeme, u32 line, u32 column, u32 size) noexcept
    {
      assert_true("Integer overflow!", tokens_info.size() < std::numeric_limits<u32>::max());
#ifdef COLT_DEBUG
      tokens.push_back(Token{ lexeme, static_cast<u32>(tokens_info.size()), buffer_id });
      tokens_info.push_back(TokenInfo{ column, size, line, line });      
#else
      tokens.push_back(Token{ lexeme, static_cast<u32>(tokens_info.size()) });
      tokens_info.push_back(TokenInfo{ column, size, line, line });
#endif // COLT_DEBUG
    }

    TokenRange getRangeFrom(Token start) const noexcept
    {
      assert_true("Invalid range!", start == Lexeme::TKN_EOF);
#ifdef COLT_DEBUG
      owns(start);
      return TokenRange{ start.info_index, start.info_index + 1, buffer_id };
#else
      return TokenRange{ start.info_index, end.info_index };
#endif // COLT_DEBUG
    }

    TokenRange getRangeFrom(Token start, Token end) const noexcept
    {
      assert_true("Invalid range!", start.info_index <= end.info_index);
#ifdef COLT_DEBUG
      owns(start), owns(end);
      return TokenRange{ start.info_index, end.info_index, buffer_id };
#else
      return TokenRange{ start.info_index, end.info_index };
#endif // COLT_DEBUG
    }

    void addIdentifier(StringView value, Lexeme lexeme, u32 line, u32 column, u32 size) noexcept
    {
      u64 ret = identifiers.size();
      identifiers.push_back(value);
      assert_true("Integer overflow!", ret <= std::numeric_limits<u32>::max());
 
#ifdef COLT_DEBUG
      tokens.push_back(Token{ lexeme, static_cast<u32>(tokens_info.size()), buffer_id, static_cast<u32>(ret) });
      tokens_info.push_back(TokenInfo{ column, size, line, line });
#else
      tokens.push_back(Token{ lexeme, static_cast<u32>(tokens_info.size()), static_cast<u32>(ret) });
      tokens_info.push_back(TokenInfo{ column, size, line, line });
#endif // COLT_DEBUG
    }
    
    void addLiteral(QWORD_t value, Lexeme lexeme, u32 line, u32 column, u32 size) noexcept
    {
      u64 ret = nb_literals.size();
      nb_literals.push_back(value);
      assert_true("Integer overflow!", ret <= std::numeric_limits<u32>::max());
 
#ifdef COLT_DEBUG
      tokens.push_back(Token{ lexeme, static_cast<u32>(tokens_info.size()), buffer_id, static_cast<u32>(ret) });
      tokens_info.push_back(TokenInfo{ column, size, line, line });
#else
      tokens.push_back(Token{ lexeme, static_cast<u32>(tokens_info.size()), static_cast<u32>(ret) });
      tokens_info.push_back(TokenInfo{ column, size, line, line });
#endif // COLT_DEBUG
    }    

    /// @brief Returns a StringView over the line in which the token appears
    /// @param tkn The Token whose line to return
    /// @return The line in which appears the token
    StringView getLineStr(Token tkn) const noexcept
    {
      owns(tkn);
      return lines[tokens_info[tkn.info_index].line_start];
    }

    /// @brief Returns the line number on which the token appears
    /// @param tkn The Token whose line to return
    /// @return The line in which appears the token (1-based)
    u32 getLine(Token tkn) const noexcept
    {
      owns(tkn);
      return tokens_info[tkn.info_index].line_start + 1;
    }

    /// @brief Returns the column on which the token appears
    /// @param tkn The Token whose column to return
    /// @return The column to return (1-based)
    u32 getColumn(Token tkn) const noexcept
    {
      owns(tkn);
      return tokens_info[tkn.info_index].column + 1;
    }

    StringView getIdentifier(Token tkn) const noexcept
    {
      assert_true("Token does not represent an identifier!", tkn.getLexeme() == Lexeme::TKN_IDENTIFIER);
      owns(tkn);
      return identifiers[tkn.literal_index];
    }

    QWORD_t getLiteral(Token tkn) const noexcept
    {
      assert_true("Token does not represent a literal!", isLiteralToken(tkn.getLexeme()), tkn.getLexeme() != Lexeme::TKN_STRING_L);
      owns(tkn);
      return nb_literals[tkn.literal_index];
    }

    /// @brief Returns the size of the Token
    /// @param tkn The Token whose size to return
    /// @return The column to return (1-based)
    TokenInfo getInfo(Token tkn) const noexcept
    {
      owns(tkn);
      return tokens_info[tkn.info_index];
    }

    /// @brief Constructs a source information from a token range
    /// @param range The token range
    /// @return SourceInfo represented by the token range
    SourceInfo makeSourceInfo(TokenRange range) const noexcept
    {
      owns(range);
      auto& tkn1_info = tokens_info[range.start_index];
      auto& tkn2_info = tokens_info[range.end_index - 1];
      auto line = StringView{ lines[tkn1_info.line_start].data(),
        lines[tkn2_info.line_end].data() + lines[tkn2_info.line_end].size() };
      auto expr = StringView{ lines[tkn1_info.line_start].data() + tkn1_info.column,
        lines[tkn2_info.line_end].data() + tkn2_info.size + tkn2_info.column };
      return SourceInfo{ tkn1_info.line_start + 1, tkn2_info.line_end + 1, expr, line };
    }

    /// @brief Constructs a source information from a token
    /// @param tkn The token
    /// @return SourceInfo represented by the token
    SourceInfo makeSourceInfo(Token tkn) const noexcept
    {
      owns(tkn);
      auto& tkn_info = tokens_info[tkn.info_index];
      auto expr = StringView{ lines[tkn_info.line_start].data() + tkn_info.column,
        lines[tkn_info.line_end].data() + tkn_info.size + tkn_info.column};
      return SourceInfo{ tkn_info.line_start + 1, expr, lines[tkn_info.line_start] };
    }

    /// @brief Returns the list of tokens
    /// @return List of tokens
    auto& getTokens() const noexcept { return tokens; }

    /// @brief Returns the list of lines
    /// @return The list of lines
    auto& getLines() const noexcept { return lines; }
  };
}

#endif // !HG_COLT_TOKEN_BUFFER