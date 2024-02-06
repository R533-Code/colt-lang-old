#ifndef HG_COLT_LEXER
#define HG_COLT_LEXER

#include "structs/list.h"
#include "structs/set.h"
#include "colt_operators.h"
#include "err/error_reporter.h"

namespace clt::lng
{
  class TokenBuffer;

  /// @brief Lexes 'to_parse'
  /// @param reporter The reporter used to generate error/warnings/messages
  /// @param to_parse The StringView to parse
  /// @return A TokenBuffer containing parsed lexemes
  TokenBuffer Lex(ErrorReporter_t reporter, StringView to_parse) noexcept;

  /// @brief Breaks down a StringView into lines
  /// @param strv The StringView to break down into lines
  /// @param buffer The buffer where to append these lines
  void CreateLines(StringView strv, FlatList<StringView, 256>& buffer) noexcept;

  class Token
  {
    /// @brief 0-based index into the array of lines of the program
    u32 line_index;
    /// @brief 1-based
    u32 column_offset;
    /// @brief 0-based index into the array of literals.
    /// The array in which to index depends on the lexeme.
    u32 literal_index;
    /// @brief The actual lexeme
    Lexeme lexeme;

#ifdef COLT_DEBUG
    /// @brief On debug, we store the ID of the TokenBuffer owning the Token
    u32 buffer_id;

    constexpr Token(Lexeme lexeme, u32 line, u32 column, u32 buffer_id, u32 literal = 0) noexcept
      : line_index(line), column_offset(column), literal_index(literal), lexeme(lexeme), buffer_id(buffer_id) {}

#else
    constexpr Token(Lexeme lexeme, u32 line, u32 column, u32 literal = 0) noexcept
      : line_index(line), column_offset(column), literal_index(literal), lexeme(lexeme) {}
#endif // COLT_DEBUG


  public:
    friend class TokenBuffer;

    Token() = delete;
    constexpr Token(Token&&) noexcept = default;
    constexpr Token(const Token&) noexcept = default;
    constexpr Token& operator=(Token&&) noexcept = default;
    constexpr Token& operator=(const Token&) noexcept = default;

    /// @brief Converts a Token to the Lexeme it represents
    constexpr operator Lexeme() const noexcept { return lexeme; }
  };

  class TokenBuffer
  {
    /// @brief The array of lines
    FlatList<StringView, 256>         lines{};
    /// @brief The array of string literals
    FlatList<UniquePtr<String>, 256>  str_literals{};
    /// @brief The array of literal numbers (including f32/f64)
    FlatList<QWORD_t, 256>            nb_literals{};
    /// @brief The array of tokens
    FlatList<Token, 512>              tokens{};

#ifdef COLT_DEBUG
    /// @brief 
    static std::atomic<u32> ID_GENERATOR;

    u32 buffer_id;
#endif // COLT_DEBUG

    /// @brief Check if a Token is owned by the current TokenBuffer
    constexpr void owns(Token tkn) const noexcept
    {
      if constexpr (isDebugBuild())
        assert_true("Token is not owned by this TokenBuffer!", tkn.buffer_id == buffer_id);
    }
    
    /// @brief Generates a Token
    /// @param lexeme The lexeme of the Token
    /// @param line The line number
    /// @param column The column of the line
    /// @param literal The literal index
    /// @return Generated Token
    void addToken(Lexeme lexeme, u32 line, u32 column, u32 literal = 0) noexcept
    {
      if constexpr (isDebugBuild())
        tokens.push_back(Token{ lexeme, line, column, buffer_id, literal });
      else
        tokens.push_back(Token{ lexeme, line, column, literal });
    }

    /// @brief Adds a line
    /// @param line The line to save
    void addLine(StringView line) noexcept
    {
      lines.push_back(line);
    }

    // Friend declaration to use addToken
    friend TokenBuffer Lex(ErrorReporter_t reporter, StringView to_parse) noexcept;

  public:
    /// @brief Default constructor
    TokenBuffer() noexcept
    {
      if constexpr (isDebugBuild())
        buffer_id = ID_GENERATOR.fetch_add(1, std::memory_order_acq_rel);
    }

    TokenBuffer(const TokenBuffer&) = delete;
    TokenBuffer& operator=(const TokenBuffer&) = delete;
    
    TokenBuffer(TokenBuffer&&) noexcept = default;
    TokenBuffer& operator=(TokenBuffer&&) noexcept = default;

    /// @brief Returns a StringView over the line in which the token appears
    /// @param tkn The Token whose line to return
    /// @return The line in which appears the token
    StringView getLineStr(Token tkn) const noexcept
    {
      owns(tkn);
      return lines[tkn.line_index];
    }
    
    /// @brief Returns the line number on which the token appears
    /// @param tkn The Token whose line to return
    /// @return The line in which appears the token (1-based)
    u32 getLine(Token tkn) const noexcept
    {
      owns(tkn);
      return tkn.line_index + 1;
    }
    
    /// @brief Returns the column on which the token appears
    /// @param tkn The Token whose column to return
    /// @return The column to return (1-based)
    u32 getColumn(Token tkn) const noexcept
    {
      owns(tkn);
      return tkn.column_offset;
    }
  };
}

#endif // !HG_COLT_LEXER
