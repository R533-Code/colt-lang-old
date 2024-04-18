#ifndef HG_COLT_AST
#define HG_COLT_AST

#include "parsed_program.h"
#include "parsed_unit.h"
#include "colt_expr.h"

namespace clt::lng
{
  void MakeAST(ParsedUnit& unit) noexcept;

  /// @brief Flags for checking if a variable is initialized or not
  enum class VarStateFlag
    : u8
  {
    /// @brief The variable is not initialized
    UNDEF         = 0b0000'0001,
    /// @brief The variable is initialized
    INIT          = 0b0000'0010,
    /// @brief The variable is moved from
    MOVED         = 0b0000'0100,
    /// @brief The variable is not initialized in all branches
    PARTIAL_UINIT = 0b0000'0011,
    /// @brief The variable is not moved in all branches
    PARTIAL_MOVE  = 0b0000'0110,
    /// @brief The variable is not initialized and was partially moved from
    PARTIAL_UMOVE = 0b0000'0101
  };

  /// @brief Merges two variable state flag
  /// @param a The first state
  /// @param b The second state
  /// @return Merge of both states
  constexpr VarStateFlag MergeStateFlag(VarStateFlag a, VarStateFlag b) noexcept
  {
    auto res = static_cast<VarStateFlag>((u8)a | (u8)b);
    assert_true("Invalid result! Wrong use of VarStateFlag!", std::popcount((u8)res) < 3);
    return res;
  }

  /// @brief Contains information about partially initialized variables
  struct PartialStateInfo
  {
    /// @brief The if branch
    StmtExprToken if_branch;
    /// @brief The else branch
    OptTok<StmtExprToken> else_branch;
    /// @brief True if the 'if' branch is not initialized
    bool is_if_uninit;
  };

  /// @brief The local variable information
  struct LocalVarInfo
  {
    /// @brief The name of the variable
    StringView name;
    /// @brief The declaration of the variable
    VarDeclExpr& decl;
    /// @brief The state of the variable
    VarStateFlag state;
  };

  struct ASTMaker
  {
    /// @brief The current token offset
    u32 current_tkn = 0;
    
    /// @brief True if parsing a private section
    u8 is_private : 1 = true;
    /// @brief The current function being parsed
    FnGlobal* current_fn = nullptr;

    /// @brief The unit that is being parsed
    ParsedUnit& to_parse;
    /// @brief The local variable table
    Vector<LocalVarInfo> local_var_table = {};

    /*-------------------------
     | MODULE RELATED MEMBERS |
     -------------------------*/

    /// @brief Contains the 'using module' declarations
    Vector<Module*> using_modules{};
    /// @brief Contains the module in which to lookup.
    /// When parsing an identifier, if MODULE1::MODULE2::IDENTIFIER
    /// is parsed, then force_lookup should contain 0:MODULE1 1:MODULE2.
    /// The `lookup` method will then use these information.
    ModuleName forced_lookup = ModuleName::getGlobalModule();
    
    /*------------------
     | USEFUL GETTERS  |
     ------------------*/

    /// @brief Returns the current reporter
    /// @return The error reporter
    const ErrorReporter& getReporter() const noexcept { return to_parse.getReporter(); }
    /// @brief Returns the current reporter
    /// @return The error reporter
    ErrorReporter& getReporter() noexcept { return to_parse.getReporter(); }

    /// @brief Returns the set of string literals
    /// @return Set of string literals
    StableSet<String>& getStrLiterals() noexcept { return to_parse.getProgram().getStrLiterals(); }
    /// @brief Returns the set of string literals
    /// @return Set of string literals
    const StableSet<String>& getStrLiterals() const noexcept { return to_parse.getProgram().getStrLiterals(); }

    /// @brief Returns the token buffer representing the parsed file
    /// @return The token buffer representing the parsed file
    const TokenBuffer& getTokenBuffer() const noexcept { return to_parse.getTokenBuffer(); }
    /// @brief Returns the token buffer representing the parsed file
    /// @return The token buffer representing the parsed file
    TokenBuffer& getTokenBuffer() noexcept { return to_parse.getTokenBuffer(); }

    /// @brief Returns the expression buffer representing the parsed file
    /// @return The expression buffer representing the parsed file
    const ExprBuffer& getExprBuffer() const noexcept { return to_parse.getExprBuffer(); }
    /// @brief Returns the expression buffer representing the parsed file
    /// @return The expression buffer representing the parsed file
    ExprBuffer& getExprBuffer() noexcept { return to_parse.getExprBuffer(); }

    /*---------------------
     | LEXEMES AND TOKENS |
     ---------------------*/

    /// @brief Returns the current token
    /// @return The current token
    Token current() const noexcept { return to_parse.getTokenBuffer().getTokens()[current_tkn]; }
    /// @brief Advances to the next token    
    void consume_current() noexcept
    {
      assert_true("Already reached EOF!", current() == Lexeme::TKN_EOF);
      ++current_tkn;
    }

    /// @brief Helper to generate TokenRange
    class TokenRangeGenerator
    {
      /// @brief The ASTMaker whose data to use to generate the range
      const ASTMaker& ast;
      /// @brief The starting offset
      Token current;

    public:
      /// @brief Constructor
      /// @param ast The ASTMaker whose state to use to generate the range
      TokenRangeGenerator(const ASTMaker& ast) noexcept
        : ast(ast), current(ast.current()) {}

      /// @brief Creates a token range from the current ASTMaker state.
      /// The range extends from the token contained in the ASTMaker when
      /// TokenRangeGenerator is constructed to the current token
      /// in the ASTMaker (non-inclusive)
      /// @return The TokenRange
      TokenRange getRange() const noexcept
      {
        return ast.getTokenBuffer().getRangeFrom(current, ast.current());
      }
    };

    /// @brief Starts a range of tokens.
    /// Call getRange() on the result to generate the TokenRange
    /// @return The range generator.
    TokenRangeGenerator startRange() const noexcept { return { *this }; }

    /*------------------
     | ERROR REPORTING |
     ------------------*/

    /// @brief Type of methods consuming tokens
    using panic_consume_t = void(ASTMaker::*)() noexcept;

    /// @brief Enum used to specify output type (for 'generate')
    enum report_as
    {
      /// @brief Generates an error
      ERROR,
      /// @brief Generates a warning
      WARNING,
      /// @brief Generates a message
      MESSAGE
    };

    template<report_as AS, typename... Args>
    void report(TokenRange range, panic_consume_t consume, io::fmt_str<Args...> fmt, Args&&... args) noexcept;

    template<report_as AS, typename... Args>
    void report(Token tkn, panic_consume_t consume, io::fmt_str<Args...> fmt, Args&&... args) noexcept;

    template<report_as AS, typename... Args>
    void report_current(panic_consume_t consume, io::fmt_str<Args...> fmt, Args&&... args) noexcept;

    template<typename... Args>
    ErrorFlag check_consume(Lexeme expected, io::fmt_str<Args...> fmt, Args&&... args) noexcept;

    template<typename... Args>
    ErrorFlag check_consume(Lexeme expected, panic_consume_t consume, io::fmt_str<Args...> fmt, Args&&... args) noexcept;

    template<typename... Args>
    bool is_current_one_of(Args&&... args) const noexcept;

    /*----------------------
     | CONSUMING FUNCTIONS |
     ----------------------*/

    /// @brief Consumes all tokens till a semicolon is hit
    void panic_consume_semicolon() noexcept;

    /*--------------------
     | PARSING FUNCTIONS |
     --------------------*/

    /// @brief Parses a primary expression.
    /// A primary expression is a read from a variable, a literal,
    /// unary or binary expression, or function call.
    /// @param accepts_conv True if the primary expression can be followed by a 'as' or 'bit_as' conversion
    /// @return Parsed expression
    ProdExprToken parse_primary(bool accepts_conv = true);

    /// @brief Parses a unary expression.
    /// A unary expression is a unary operator (!, ~, *, &) applied
    /// to a primary expression. It can result also result
    /// in a AddressOf and PtrRead expression.
    /// @return Parsed expression
    ProdExprToken parse_unary();   

    template<Lexeme begin, Lexeme end, typename RetT, typename... Args>
    /// @brief Parses an expression that enclosed by 'begin' and 'end'
    /// @tparam begin The starting lexeme
    /// @tparam end The end lexeme
    /// @tparam RetT The return type of the parsing function
    /// @tparam Args... The arguments taken by the parsing function
    /// @param start_error The error to print if the starting token is missing
    /// @param end_error The error to print if the end token is missing
    /// @param panic The panic consumer to call on errors
    /// @param method_ptr The method pointer that parses the expression enclosed
    /// @param args... The arguments to forward to 'method_ptr'
    /// @return The return of 'method_ptr'
    RetT parse_enclosed(io::fmt_str<> start_error, io::fmt_str<> end_error, panic_consume_t panic, RetT(ASTMaker::* method_ptr)(Args...), Args&&... args) noexcept;

    template<typename RetT, typename... Args>
    /// @brief Parses an expression enclosed in parenthesis
    /// @tparam RetT The return type of the parsing function
    /// @tparam Args... The arguments taken by the parsing function
    /// @param method_ptr The method pointer that parses the expression enclosed
    /// @param args... The arguments to forward to 'method_ptr'
    RetT parse_parenthesis(RetT(ASTMaker::* method_ptr)(Args...), Args&&... args) noexcept;
    
    template<typename RetT, typename... Args>
    /// @brief Parses an expression enclosed in parenthesis
    /// @tparam RetT The return type of the parsing function
    /// @tparam Args... The arguments taken by the parsing function
    /// @param consume The panic consumer to call on errors
    /// @param method_ptr The method pointer that parses the expression enclosed
    /// @param args... The arguments to forward to 'method_ptr'
    RetT parse_parenthesis(panic_consume_t consume, RetT(ASTMaker::* method_ptr)(Args...), Args&&... args) noexcept;

    /// @brief Convert a read from a variable to a declaration
    /// @param expr The expression (can be any expression)
    /// @return None or VarDeclExpr or GlobalDeclExpr
    OptTok<StmtExprToken> decl_from_read(ProdExprToken expr) const noexcept;
  };
  
  template<ASTMaker::report_as AS, typename ...Args>
  void ASTMaker::report(TokenRange range, panic_consume_t consume, io::fmt_str<Args...> fmt, Args&&... args) noexcept
  {
    auto source_info = getTokenBuffer().makeSourceInfo(range);
    StringView str;
    if constexpr (sizeof...(Args) == 0)
      str = fmt;
    else
      str = getReporter().fmt(fmt, std::forward<Args>(args)...);

    if constexpr (AS == ERROR)
      getReporter().error(str, source_info);
    if constexpr (AS == WARNING)
      getReporter().error(str, source_info);
    if constexpr (AS == MESSAGE)
      getReporter().error(str, source_info);
    if (consume != nullptr)
      (*this.*consume)();
  }

  template<ASTMaker::report_as AS, typename ...Args>
  void ASTMaker::report(Token tkn, panic_consume_t consume, io::fmt_str<Args...> fmt, Args && ...args) noexcept
  {
    auto source_info = getTokenBuffer().makeSourceInfo(tkn);
    StringView str;
    if constexpr (sizeof...(Args) == 0)
      str = fmt;
    else
      str = getReporter().fmt(fmt, std::forward<Args>(args)...);

    if constexpr (AS == ERROR)
      getReporter().error(str, source_info);
    if constexpr (AS == WARNING)
      getReporter().error(str, source_info);
    if constexpr (AS == MESSAGE)
      getReporter().error(str, source_info);
    if (consume != nullptr)
      (*this.*consume)();
  }

  template<ASTMaker::report_as AS, typename ...Args>
  void ASTMaker::report_current(panic_consume_t consume, io::fmt_str<Args...> fmt, Args&&... args) noexcept
  {
    report<AS>(current(), consume, fmt, std::forward<Args>(args)...);
  }

  template<typename ...Args>
  ErrorFlag ASTMaker::check_consume(Lexeme expected, panic_consume_t consume, io::fmt_str<Args...> fmt, Args && ...args) noexcept
  {
    if (current() == expected)
    {
      consume_current();
      return ErrorFlag::success();
    }
    report<ERROR>(current(), consume, fmt, std::forward<Args>(args)...);
    return ErrorFlag::error();
  }

  template<typename ...Args>
  bool ASTMaker::is_current_one_of(Args&&... args) const noexcept
  {
    auto tkn = current();
    return (... || (tkn == args));
  }

  template<typename ...Args>
  ErrorFlag ASTMaker::check_consume(Lexeme expected, io::fmt_str<Args...> fmt, Args&& ...args) noexcept
  {
    return check_consume(expected, nullptr, fmt, std::forward<Args>(args)...);
  }  

  template<Lexeme begin, Lexeme end, typename RetT, typename ...Args>
  RetT ASTMaker::parse_enclosed(io::fmt_str<> start_error, io::fmt_str<> end_error, panic_consume_t panic, RetT(ASTMaker::* method_ptr)(Args...), Args && ...args) noexcept
  {
    auto start = current();
    check_consume(begin, start_error).discard();
    if constexpr (std::is_same_v<RetT, void>)
    {
      (*this.*method_ptr)(std::forward<Args>(args)...);
      if (current() != end)
        report<ERROR>(start, panic, end_error);
      else
        consume_current();
    }
    else
    {
      auto to_ret = (*this.*method_ptr)(std::forward<Args>(args)...);
      if (current() != end)
        report<ERROR>(start, panic, end_error);
      else
        consume_current();
      return to_ret;
    }
  }

  template<typename RetT, typename... Args>
  RetT ASTMaker::parse_parenthesis(RetT(ASTMaker::* method_ptr)(Args...), Args&&... args) noexcept
  {
    return parse_enclosed<Token::TKN_LEFT_PAREN, Token::TKN_RIGHT_PAREN>("Expected a '(!", "Expected a ')!", nullptr, method_ptr, std::forward<Args>(args)...);
  }
  
  template<typename RetT, typename... Args>
  RetT ASTMaker::parse_parenthesis(panic_consume_t consume, RetT(ASTMaker::* method_ptr)(Args...), Args&&... args) noexcept
  {
    return parse_enclosed<Token::TKN_LEFT_PAREN, Token::TKN_RIGHT_PAREN>("Expected a '(!", "Expected a ')!", consume, method_ptr, std::forward<Args>(args)...);
  }
}

#endif // !HG_COLT_AST
