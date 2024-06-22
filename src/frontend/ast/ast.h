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
#include "run/qword_op.h"

namespace clt::lng
{
  /// @brief Generates the AST and stores the result in 'unit'
  /// @param unit The unit whose AST to generate
  /// @pre !unit.isParsed()
  void make_ast(ParsedUnit& unit) noexcept;

  /// @brief Prints an expression (for debugging purposes)
  /// @param tkn The token to print
  /// @param unit The unit owning the expression
  /// @param depth The depth (for spacing)
  void print_expr(ProdExprToken tkn, const ParsedUnit& unit, u64 depth = 0) noexcept;
  /// @brief Prints an expression (for debugging purposes)
  /// @param tkn The expression to print
  /// @param unit The unit owning the expression
  /// @param depth The depth (for spacing)
  void print_expr(const ExprBase* tkn, const ParsedUnit& unit, u64 depth = 0) noexcept;

  /// @brief Transforms a literal token to a built-in ID
  /// @param tkn The token
  /// @return BuiltinID equivalent of the literal token
  constexpr BuiltinID keyword_to_builtin_id(Lexeme tkn) noexcept
  {
    using enum Lexeme;
    assert_true("Token must be TKN_.*_L", isBuiltinToken(tkn));
    return static_cast<BuiltinID>(static_cast<u8>(tkn) - static_cast<u8>(TKN_KEYWORD_bool));
  }

  /// @brief Folds a binary operation
  /// @param op The operator to apply
  /// @param a The first operand
  /// @param b The second operand
  /// @param type The type of the operands
  /// @return Result of the operation
  run::ResultQWORD constant_fold(BinaryOp op, QWORD_t a, QWORD_t b, run::TypeOp type) noexcept;

  enum class ComparisonSet
  {
    /// @brief {<, <=}
    LE_OR_LEQ,
    /// @brief ==
    EQUAL,
    /// @brief {>, >=}
    GE_OR_GEQ,
    /// @brief !=
    NONE
  };

  /// @brief Returns the comparison set to which a comparison operator belongs to
  /// @param comparison The comparison operator
  /// @return The comparison set representing 'comparison'
  constexpr ComparisonSet token_to_comparison_set(Token comparison) noexcept
  {
    using enum Lexeme;
    using enum ComparisonSet;
    assert_true("Expected comparison token", isComparisonToken(comparison));
    if (comparison == TKN_EQUAL_EQUAL)
      return EQUAL;
    else if (comparison == TKN_LESS || comparison == TKN_LESS_EQUAL)
      return LE_OR_LEQ;
    else if (comparison == TKN_GREAT || comparison == TKN_GREAT_EQUAL)
      return GE_OR_GEQ;
    return NONE;
  }

  constexpr StringView to_str(ComparisonSet set) noexcept
  {
    using enum ComparisonSet;
    switch_no_default (set)
    {
    case LE_OR_LEQ:
      return "'<' or '<='";
    case EQUAL:
      return "'=='";
    case GE_OR_GEQ:
      return "'>' or '>='";
    case NONE:
      return "!=";
    }
  }

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
  constexpr VarStateFlag merge_state_flag(VarStateFlag a, VarStateFlag b) noexcept
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

  /// @brief Class responsible of generating the AST
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
    /// @brief The current scope being parsed
    ScopeExpr* current_scope = nullptr;
    
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

    /// @brief Constructor, does all the parsing
    /// @param unit The unit to parse
    ASTMaker(ParsedUnit& unit) noexcept
      : to_parse(unit)
    {
      auto s = scopedSetPanic(&ASTMaker::panic_consume_semicolon);
      while (current() != Lexeme::TKN_EOF)
        print_expr(parse_statement(), to_parse);
    }

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
    const ExprBuffer& Expr() const noexcept { return to_parse.getExprBuffer(); }
    /// @brief Returns the expression buffer representing the parsed file
    /// @return The expression buffer representing the parsed file
    ExprBuffer& Expr() noexcept { return to_parse.getExprBuffer(); }
    
    /// @brief Shorthand for Expr().getExpr()
    /// @param expr The expression to get
    /// @return ProdExprVariant
    ProdExprVariant& Expr(ProdExprToken expr) noexcept { return to_parse.getExprBuffer().getExpr(expr); }
    /// @brief Shorthand for Expr().getExpr()
    /// @param expr The expression to get
    /// @return ProdExprVariant
    const ProdExprVariant& Expr(ProdExprToken expr) const noexcept { return to_parse.getExprBuffer().getExpr(expr); }
    /// @brief Shorthand for Expr().getExpr()
    /// @param expr The expression to get
    /// @return StmtExprVariant
    StmtExprVariant& Expr(StmtExprToken expr) noexcept { return to_parse.getExprBuffer().getExpr(expr); }
    /// @brief Shorthand for Expr().getExpr()
    /// @param expr The expression to get
    /// @return StmtExprVariant
    const StmtExprVariant& Expr(StmtExprToken expr) const noexcept { return to_parse.getExprBuffer().getExpr(expr); }
    /// @brief Shorthand for Expr().getType()
    /// @param expr The expression whose type to get
    /// @return The type
    const TypeVariant& Type(ProdExprToken expr) const noexcept { return to_parse.getExprBuffer().getType(expr); }
    /// @brief Shorthand for Expr().getType()
    /// @param expr The expression whose type to get
    /// @return The type
    const TypeVariant& Type(TypeToken expr) const noexcept { return to_parse.getExprBuffer().getType(expr); }
    /// @brief Shorthand for Expr().getType()
    /// @param expr The expression whose type to get
    /// @return The type
    const TypeVariant& Type(const ExprBase& expr) const noexcept { return Type(expr.getType()); }
    /// @brief Shorthand for getProgram().getTypes()
    /// @return The type buffer
    TypeBuffer& Type() noexcept { return to_parse.getProgram().getTypes(); }

  private:
    /// @brief Gets a string representing the typename of 'var'
    /// @param var The type whose typename to return
    /// @return StringVoew representing the typename of 'var'
    StringView getTypeName(const TypeVariant& var) noexcept { return to_parse.getProgram().getTypes().getTypeName(var); }
    /// @brief Gets a string representing the typename of 'var'
    /// @param var The type whose typename to return
    /// @return StringVoew representing the typename of 'var'
    StringView getTypeName(TypeToken var) noexcept { return to_parse.getProgram().getTypes().getTypeName(var); }

    /*---------------------
     | LEXEMES AND TOKENS |
     ---------------------*/

    /// @brief Returns the current token
    /// @return The current token
    Token current() const noexcept { return to_parse.getTokenBuffer().getTokens()[current_tkn]; }
    /// @brief Advances to the next token    
    void consume_current() noexcept
    {
      //assert_true("Already reached EOF!", current() == Lexeme::TKN_EOF);
      current_tkn += (u8)(current() != Lexeme::TKN_EOF);
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
        //assert_true("Missing call to consume_current()!", current != ast.current());
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

    /// @brief Shorthand for getProgram().getWarnFor()
    /// @return WarnFor
    const WarnFor& getWarnFor() const noexcept { return to_parse.getProgram().getWarnFor(); }

    /// @brief Check if the AST needs to generate a warning for 'err'.
    /// This method does not accept a DIV_BY_ZERO as this is always
    /// an error, and not a warning.
    /// @param err The error
    /// @return True if using getWarnFor, the warning must be printed
    /// @pre err != DIV_BY_ZERO
    bool warnFor(run::OpError err) const noexcept;

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

    void handle_comparison_chain_error(Token comparison, ComparisonSet comparison_set) noexcept;

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
    void panic_consume() noexcept
    {
      if (current_panic)
        (this->*current_panic)();
    }

    /// @brief Consumes all tokens till a semicolon is hit, and consumes the semicolon
    void panic_consume_semicolon() noexcept
    {
      panic_consume_till<Lexeme::TKN_SEMICOLON>();
      if (current() == Lexeme::TKN_SEMICOLON)
        consume_current();
    }

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
    ProdExprToken parse_primary_literal(const TokenRangeGenerator& range) noexcept;
    
    /// @brief Handles an invalid primary expression.
    /// Avoids reporting an error if the lexer already did.
    /// This is a leaf function, and thus cannot throw `ExitRecursionExcept`.
    /// @param range The token range representing the expression
    /// @return ErrorExpr
    ProdExprToken parse_primary_invalid(const TokenRangeGenerator& range) noexcept;

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
    ProdExprToken parse_unary_and(ProdExprToken child, const TokenRangeGenerator& range) noexcept;
    
    /// @brief Handles a PtrLoad expression
    /// This is a leaf function, and thus cannot throw `ExitRecursionExcept`.
    /// @param child The child from which to load
    /// @return AddressOfExpr or ErrorExpr if 'child' is not a pointer
    /// @pre The previously parsed unary operator must be '*'
    ProdExprToken parse_unary_star(ProdExprToken child, const TokenRangeGenerator& range) noexcept;

    ProdExprToken parse_binary();

    ProdExprToken parse_binary_condition();
    
    ProdExprToken parse_binary_internal(Token previous);

    ProdExprToken parse_conversion(ProdExprToken to_conv, const TokenRangeGenerator& range);

    ProdExprToken parse_assignment(ProdExprToken assign_to, const TokenRangeGenerator& range);
    
    ProdExprToken parse_comparison(Token comparison, ProdExprToken lhs, const TokenRangeGenerator& range);
    
    ProdExprToken parse_comparison(Token comparison, ProdExprToken lhs, ProdExprToken rhs, const TokenRangeGenerator& range);

    StmtExprToken parse_scope(bool accepts_single = true);

    StmtExprToken parse_var_decl(bool is_global = false);

    OptTok<StmtExprToken> parse_condition(bool is_elif = false);

    ExprBase* parse_statement();

    TypeToken parse_typename() noexcept;

    ErrorFlag parse_local_var_mutability(bool& is_mut) noexcept;
    
    ErrorFlag parse_global_var_mutability(bool& is_mut) noexcept;

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

    /*----------------
     | MAKE HELPERS  |
     ---------------*/

    /// @brief Verify that a type supports an operator and creates a BinaryExpr.
    /// This method is also responsible of constant folding.
    /// @param range The range of tokens representing the expression
    /// @param lhs The left hand side
    /// @param op The operator
    /// @param rhs The right hand side
    /// @return ErrorExpr or BinaryExpr
    ProdExprToken makeBinary(TokenRange range, ProdExprToken lhs, BinaryOp op, ProdExprToken rhs) noexcept;
    /// @brief Verify that a type supports an operator and creates a UnaryExpr.
    /// This method is also responsible of constant folding
    /// @param range The range of tokens representing the expression
    /// @param op The operator
    /// @param child The expression on which the operator is applied
    /// @return ErrorExpr or UnaryExpr
    ProdExprToken makeUnary(TokenRange range, UnaryOp op, ProdExprToken child) noexcept;
    
    /// @brief Verify that a type supports a conversion and creates a CastExpr
    /// @param range The range of tokens representing the expression
    /// @param to_cast The expression to cast
    /// @param to The type to cast to
    /// @param is_bit_cast True if bit cast
    /// @return CastExpr or ErrorExpr
    ProdExprToken makeCast(TokenRange range, ProdExprToken to_cast, TypeToken to, bool is_bit_cast) noexcept;

    /// @brief Creates a condition expression.
    /// This method is also responsible of constant folding, which
    /// could mean that an if without an else could be optimized to nothing.
    /// @param range The range of tokens representing the expression
    /// @param condition The if condition
    /// @param if_stmt The if statement
    /// @param else_stmt The else statement or None for no else
    /// @return None if the whole condition was optimized away or ConditionExpr
    OptTok<StmtExprToken> makeCondition(TokenRange range, ProdExprToken condition,
      StmtExprToken if_stmt, OptTok<StmtExprToken> else_stmt) noexcept;

    /// @brief Constant folds two literals using 'op' as the binary operator.
    /// This method will print warnings following 'WarnAll'
    /// @param range The range of tokens representing the expression
    /// @param lhs The left hand side of the expression
    /// @param op The operator
    /// @param rhs The right hand side of the expression
    /// @return LiteralExpr or ErrorExpr
    ProdExprToken constantFold(TokenRange range, const LiteralExpr& lhs, BinaryOp op, const LiteralExpr& rhs) noexcept;
    /// @brief Constant folds a literal using 'op' as the unary operator.
    /// This method will print warnings following 'WarnAll'
    /// @param range The range of tokens representing the expression
    /// @param op The operator
    /// @param lhs The expression on which the operator is applied
    /// @return LiteralExpr
    ProdExprToken constantFold(TokenRange range, UnaryOp op, const LiteralExpr& lhs) noexcept;

    /// @brief Constant folds a cast
    /// This method will print warnings following 'WarnAll'
    /// @param range The range of tokens representing the expression
    /// @param to_conv The literal to convert
    /// @param to The type to convert to
    /// @return LiteralExpr
    ProdExprToken constantFold(TokenRange range, const LiteralExpr& to_conv, const BuiltinType& to) noexcept;    

    /// @brief Check if 'expr' represents a LiteralExpr with value 0
    /// @param expr The expression whose value to check
    /// @return True if the expr is a LiteralExpr with value 0
    bool isLiteralZero(ProdExprToken expr) const noexcept;

    bool isInvalidChain(ComparisonSet old, ComparisonSet new_set) const noexcept;
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
      getReporter().warn(str, source_info);
    if constexpr (AS == MESSAGE)
      getReporter().message(str, source_info);
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
      getReporter().warn(str, source_info);
    if constexpr (AS == MESSAGE)
      getReporter().message(str, source_info);
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
