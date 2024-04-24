/*****************************************************************//**
 * @file   ast.h
 * @brief  Contains the Abstract Syntax Tree related codes.
 * 
 * @author RPC
 * @date   April 2024
 *********************************************************************/
#ifndef HG_COLT_AST
#define HG_COLT_AST

#include "parsed_program.h"
#include "parsed_unit.h"
#include "colt_expr.h"
#include "util/exit_recursion.h"

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

  template<typename T>
  /// @brief Represents an assignment that will restore the previous value
  /// of a variable at the end of the scope
  class ScopedAssignment
  {
    /// @brief The previous value
    T previous;
    /// @brief The variable to assign to
    T& to_assign;

  public:
    template<typename Ty>
    /// @brief Constructor
    /// @param to_assign The value to assign to, whose previous to save
    /// @param value The value to assign for the current scope
    ScopedAssignment(T& to_assign, Ty&& value) noexcept
      : previous(std::move(to_assign)), to_assign(to_assign)
    {
      to_assign = std::forward<Ty>(value);
    }

    /// @brief Restores the previous value of 'to_assign'
    ~ScopedAssignment()
    {
      to_assign = std::move(to_assign);
    }
  };

  class ASTMaker
  {
    /// @brief Type of methods consuming tokens
    using panic_consume_t = void(ASTMaker::*)() noexcept;

    /// @brief The unit that is being parsed
    ParsedUnit& to_parse;

    /// @brief The current token offset
    u32 current_tkn = 0;
    /// @brief The recursion depth
    u16 recurse_depth = 0;    
    /// @brief True if parsing a private section
    u8 is_private : 1 = true;
    /// @brief The current function being parsed
    FnGlobal* current_fn = nullptr;
    
    /// @brief The local variable table
    Vector<LocalVarInfo> local_var_table = {};

    /*-------------------------
     | MODULE RELATED MEMBERS |
     -------------------------*/

    /// @brief Contains the 'using module' declarations
    Vector<Module*> using_modules = {};
    /// @brief Contains the module in which to lookup.
    /// When parsing an identifier, if MODULE1::MODULE2::IDENTIFIER
    /// is parsed, then force_lookup should contain 0:MODULE1 1:MODULE2.
    /// The `lookup` method will then use these information.
    ModuleName forced_lookup = ModuleName::getGlobalModule();
    /// @brief The current panic function
    panic_consume_t current_panic = nullptr;
  
  public:
    /// @brief The maximum recursion depth allowed
    static constexpr u16 MAX_RECURSION_DEPTH = 256;

    /// @brief Constructor
    /// @param unit The unit to parse
    ASTMaker(ParsedUnit& unit) noexcept
      : to_parse(unit) {}

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



  private:

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
      MAKE_DELETE_COPY_AND_MOVE_FOR(TokenRangeGenerator);

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

    /// @brief Registers 'new_value' as the current panic function
    /// for the current scope.
    /// @param new_value The new panic function
    /// @return RAII helper
    ScopedAssignment<panic_consume_t> scopedSetPanic(panic_consume_t new_value) noexcept
    {
      return { current_panic, new_value };
    }

    /// @brief Helper to check for recursion depth
    class RecursionDepthChecker
    {
      /// @brief The ASTMaker whose data to use
      ASTMaker& ast;

    public:
      MAKE_DELETE_COPY_AND_MOVE_FOR(RecursionDepthChecker);
      
      /// @brief Constructs a checker, see `addDepth`
      /// @param ast The ASTMaker whose data to use
      RecursionDepthChecker(ASTMaker& ast)
        : ast(ast)
      {
        ast.recurse_depth++;
        if (ast.recurse_depth == ASTMaker::MAX_RECURSION_DEPTH)
        {
          ast.recurse_depth = 0;
          ast.getReporter().error("Exceeded recursion depth!");
          throw ExitRecursionExcept();
        }
      }

      /// @brief Destructor, restores the recursion depth to its previous value
      ~RecursionDepthChecker() noexcept
      {
        ast.recurse_depth--;
      }
    };

    /// @brief Returns an object responsible of checking for recursion depth.
    /// When constructed, the object increments the 'recurse_depth' member of the
    /// ASTMaker. On destruction, the object decrements 'recurse_depth'.
    /// If 'recurse_depth' equals to MAX_RECURSION_DEPTH, an ExitRecursionExcept
    /// is thrown.
    /// Care must be taken: the function calling this method must not be noexcept.   
    /// @return RecursionDepthChecker
    RecursionDepthChecker addDepth() { return { *this }; }

    /*------------------
     | ERROR REPORTING |
     ------------------*/    

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
    /// @brief Reports a message/warning/error
    /// @tparam AS How to report the message (can be any of ERROR, WARNING, MESSAGE)
    /// @tparam Args... The arguments to format
    /// @param range The range of tokens to highlight
    /// @param consume The panic method to call (can be null)
    /// @param fmt The message/warning/error format
    /// @param args... The arguments to format
    void report(TokenRange range, panic_consume_t consume, io::fmt_str<Args...> fmt, Args&&... args) noexcept;

    template<report_as AS, typename... Args>
    /// @brief Reports a message/warning/error
    /// @tparam AS How to report the message (can be any of ERROR, WARNING, MESSAGE)
    /// @tparam Args... The arguments to format
    /// @param tkn The token to highlight
    /// @param consume The panic method to call (can be null)
    /// @param fmt The message/warning/error format
    /// @param args... The arguments to format
    void report(Token tkn, panic_consume_t consume, io::fmt_str<Args...> fmt, Args&&... args) noexcept;

    template<report_as AS, typename... Args>
    /// @brief Reports a message/warning/error for the current token
    /// @tparam AS How to report the message (can be any of ERROR, WARNING, MESSAGE)
    /// @tparam Args... The arguments to format
    /// @param consume The panic method to call (can be null)
    /// @param fmt The message/warning/error format
    /// @param args... The arguments to format
    void report_current(panic_consume_t consume, io::fmt_str<Args...> fmt, Args&&... args) noexcept;

    template<typename... Args>
    ErrorFlag check_consume(Lexeme expected, io::fmt_str<Args...> fmt, Args&&... args) noexcept;

    template<typename... Args>
    ErrorFlag check_consume(Lexeme expected, panic_consume_t consume, io::fmt_str<Args...> fmt, Args&&... args) noexcept;

    template<typename... Args>
    /// @brief Check if the current token is any of 'args...'
    /// @param args... The token to compare against
    /// @return True if the current token is equal to any of 'args...'
    bool is_current_one_of(Args&&... args) const noexcept;

    /*----------------------
     | CONSUMING FUNCTIONS |
     ----------------------*/

    /**
    * ALL OF THE CONSUME METHODS MUST BE IDEMPOTENT: calling
    * the same method more than once won't do anything.
    */

    template<Lexeme TILL>
    /// @brief Consumes all tokens till 'TILL' (or EOF) is hit
    /// @tparam TILL The lexeme to consume to
    void panic_consume_till() noexcept;

    /// @brief Calls 'current_panic' if it isn't null
    void panic_consume() noexcept { if (current_panic) (this->*current_panic)(); }

    /// @brief Consumes all tokens till a semicolon is hit
    void panic_consume_semicolon() noexcept { panic_consume_till<Lexeme::TKN_SEMICOLON>(); }

    /// @brief Consumes all tokens till a left parenthesis is hit
    void panic_consume_lparen() noexcept { panic_consume_till<Lexeme::TKN_LEFT_PAREN>(); }

    /*--------------------
     | PARSING FUNCTIONS |
     --------------------*/

    /// @brief Parses a primary expression.
    /// A primary expression is a read from a variable, a literal,
    /// unary or binary expression, or function call.
    /// @param accepts_conv True if the primary expression can be followed by a 'as' or 'bit_as' conversion
    /// @return Parsed expression
    /// @throw ExitRecursionExcept
    ProdExprToken parse_primary(bool accepts_conv = true);

    /// @brief Parses a literal token.
    /// This is a leaf function, and thus cannot throw `ExitRecursionExcept`.
    /// @param range The token range representing the expression
    /// @return LiteralExpr
    /// @pre `isLiteralToken(current())`
    ProdExprToken parse_primary_literal(TokenRange range) noexcept;
    
    /// @brief Handles an invalid primary expression.
    /// Avoids reporting an error if the lexer already did.
    /// This is a leaf function, and thus cannot throw `ExitRecursionExcept`.
    /// @param range The token range representing the expression
    /// @return ErrorExpr
    ProdExprToken parse_primary_invalid(TokenRange range) noexcept;

    /// @brief Parses a unary expression.
    /// A unary expression is a unary operator (!, ~, *, &) applied
    /// to a primary expression. It can result also result
    /// in a AddressOf and PtrRead expression.
    /// @return Parsed expression or ErrorExpr
    /// @throw ExitRecursionExcept
    ProdExprToken parse_unary();   

    /// @brief Handles an AddressOf expression
    /// This is a leaf function, and thus cannot throw `ExitRecursionExcept`.
    /// @param child The child whose address to return
    /// @return AddressOfExpr or ErrorExpr if 'child' is not a 'ReadExpr'
    /// @pre The previously parsed unary operator must be '&'
    ProdExprToken parse_unary_and(ProdExprToken child, TokenRange range) noexcept;
    
    /// @brief Handles a PtrLoad expression
    /// This is a leaf function, and thus cannot throw `ExitRecursionExcept`.
    /// @param child The child from which to load
    /// @return AddressOfExpr or ErrorExpr if 'child' is not a pointer
    /// @pre The previously parsed unary operator must be '*'
    ProdExprToken parse_unary_star(ProdExprToken child, TokenRange range) noexcept;

    ProdExprToken parse_binary(u8 precedence);

    ProdExprToken parse_conversion(ProdExprToken to_conv, const TokenRangeGenerator& range);

    ProdExprToken parse_assignment(ProdExprToken assign_to, const TokenRangeGenerator& range);
    
    ProdExprToken parse_comparison(ProdExprToken lhs, const TokenRangeGenerator& range);

    /*------------------------------
     | TEMPLATED PARSING FUNCTIONS |
     -----------------------------*/

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
    RetT parse_enclosed(io::fmt_str<> start_error, io::fmt_str<> end_error, panic_consume_t panic, RetT(ASTMaker::*method_ptr)(Args...), Args&&... args) noexcept;

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

    /*----------------
     | STATE HELPERS |
     ---------------*/

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
      str = StringView{ fmt.get().data(), fmt.get().size() };
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
      str = StringView{ fmt.get().data(), fmt.get().size() };
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
    using enum Lexeme;
    auto panic = scopedSetPanic(&ASTMaker::panic_consume_lparen);
    return parse_enclosed<TKN_LEFT_PAREN, TKN_RIGHT_PAREN>("Expected a '(!", "Expected a ')!", nullptr, method_ptr, std::forward<Args>(args)...);
  }
  
  template<typename RetT, typename... Args>
  RetT ASTMaker::parse_parenthesis(panic_consume_t consume, RetT(ASTMaker::* method_ptr)(Args...), Args&&... args) noexcept
  {
    using enum Lexeme;
    auto panic = scopedSetPanic(&ASTMaker::panic_consume_lparen);
    return parse_enclosed<TKN_LEFT_PAREN, TKN_RIGHT_PAREN>("Expected a '(!", "Expected a ')!", consume, method_ptr, std::forward<Args>(args)...);
  }

  template<Lexeme TILL>
  void ASTMaker::panic_consume_till() noexcept
  {
    using enum Lexeme;

    // Consume everything till a 'TILL' is hit
    auto tkn = current();
    while (tkn != TKN_EOF && tkn != TILL)
    {
      consume_current();
      tkn = current();
    }
  }
}

#endif // !HG_COLT_AST
