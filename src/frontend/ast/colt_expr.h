/*****************************************************************//**
 * @file   colt_expr.h
 * @brief  Contains the types used to represents the AST's nodes.
 * 
 * @author RPC
 * @date   March 2024
 *********************************************************************/
#ifndef HG_COLT_EXPR
#define HG_COLT_EXPR

#include "colt_expr_token.h"

namespace clt::lng
{
  template<typename T>
  concept ProducerExpr = meta::is_any_of<T, COLTC_PROD_EXPR_LIST>;

  template<typename T>
  concept StatementExpr = meta::is_any_of<T, COLTC_STMT_EXPR_LIST>;

  /// @brief Base class of all expressions
  class ExprBase
  {
    /// @brief Represents the type of the expression
    TypeToken _type;
    /// @brief Represents the range of Token forming the expression
    TokenRange range;
    /// @brief The ID of the expression
    ExprID expr_id;

  protected:
    /// @brief Byte that can be used for anything
    u8 padding0 = 0;
    /// @brief Byte that can be used for anything
    u8 padding1 = 0;
    /// @brief Byte that can be used for anything
    u8 padding2 = 0;

  public:
    /// @brief Constructor
    /// @param id The ID of the expression
    /// @param type The type of the expression
    /// @param range The range representing the expression
    /// @param padding0 Byte that can be used by the child class
    /// @param padding1 Byte that can be used by the child class
    /// @param padding2 Byte that can be used by the child class
    constexpr ExprBase(ExprID id, TypeToken type, TokenRange range, u8 padding0 = 0, u8 padding1 = 0, u8 padding2 = 0) noexcept
      : _type(type), range(range), expr_id(id), padding0(padding0), padding1(padding1), padding2(padding2) {}

    ExprBase() = delete;
    MAKE_DEFAULT_COPY_AND_MOVE_FOR(ExprBase);

    /// @brief Returns the range of tokens representing the expression
    /// @return Range of tokens representing the expression
    constexpr TokenRange token_range() const noexcept { return range; }
    /// @brief Sets the range of tokens representing the expression to 'nrange'
    /// @param nrange The new range
    constexpr void token_range(TokenRange nrange) noexcept { range = nrange; }
    /// @brief Returns the type of the expression.
    /// @return The type of the expression
    constexpr TypeToken type() const noexcept { return _type; }

    /// @brief Returns the expression ID
    /// @return The expression ID
    constexpr ExprID classof() const noexcept { return expr_id; }

    /// @brief Check if the current expression represents an ErrorExpr
    /// @return True if ErrorExpr
    constexpr bool is_error() const noexcept { return classof() == ExprID::EXPR_ERROR; }
  };

  /// @brief Represents an error expression.
  /// Used to avoid reporting a lot of errors.
  class ErrorExpr
    final : public ExprBase
  {
  public:
    /// @brief Constructor
    /// @param id The ID of the expression
    /// @param type The type of the expression
    /// @param range The range representing the expression
    constexpr ErrorExpr(TokenRange range, TypeToken type) noexcept
      : ExprBase(TypeToExprID<ErrorExpr>(), type, range) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(ErrorExpr);
  };
  
  /// @brief Represents an no-op expression.
  class NOPExpr
    final : public ExprBase
  {
  public:
    /// @brief Constructor
    /// @param id The ID of the expression
    /// @param type The type of the expression
    /// @param range The range representing the expression
    constexpr NOPExpr(TokenRange range, TypeToken type) noexcept
      : ExprBase(TypeToExprID<NOPExpr>(), type, range) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(NOPExpr);
  };

  /// @brief Represents a literal expression
  class LiteralExpr
    final : public ExprBase
  {
    /// @brief The value of the literal
    QWORD_t _value;

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The type of the literal
    /// @param value The literal value
    constexpr LiteralExpr(TokenRange range, TypeToken type, QWORD_t value) noexcept
      : ExprBase(TypeToExprID<LiteralExpr>(), type, range), _value(value) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(LiteralExpr);

    /// @brief Return the value of the literal
    /// @return The value of the literal
    constexpr QWORD_t value() const noexcept { return _value; }
  };

  /// @brief Represents a unary expression
  class UnaryExpr
    final : public ExprBase
  {
    /// @brief The expression on which the operation is applied
    ProdExprToken _expr;

    // The operator is stored in the padding bits of ExprBase.

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The type of the expression (same as 'expr')
    /// @param op The operation
    /// @param expr The expression on which the unary operation is applied
    constexpr UnaryExpr(TokenRange range, TypeToken type, UnaryOp op, ProdExprToken expr) noexcept
      : ExprBase(TypeToExprID<UnaryExpr>(), type, range, static_cast<u8>(op)), _expr(expr) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(UnaryExpr);

    /// @brief Returns the expression on which the unary operation is applied.
    /// @return The expression on which the unary operation is applied
    constexpr ProdExprToken expr() const noexcept { return _expr; }

    /// @brief Returns the operation applied on the expression
    /// @return Any unary op
    constexpr UnaryOp op() const noexcept { return static_cast<UnaryOp>(ExprBase::padding0); }
  };

  /// @brief Reprents a binary operation
  class BinaryExpr
    final : public ExprBase
  {
    /// @brief The left hand side of the expression
    ProdExprToken _lhs;
    /// @brief The right hand side of the expression
    ProdExprToken _rhs;

    // The operator is stored in the padding bits of ExprBase.

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The type of the expression (can differ if comparison)
    /// @param lhs The left hand side of the expression
    /// @param op The operation applied on the expression
    /// @param rhs The right hand side of the expression
    constexpr BinaryExpr(TokenRange range, TypeToken type, ProdExprToken lhs, BinaryOp op, ProdExprToken rhs) noexcept
      : ExprBase(TypeToExprID<BinaryExpr>(), type, range, static_cast<u8>(op)), _lhs(lhs), _rhs(rhs) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(BinaryExpr);

    /// @brief Returns the left hand side of the expression
    /// @return The left hand side
    constexpr ProdExprToken lhs() const noexcept { return _lhs; }
    /// @brief Returns the right hand side of the expression
    /// @return The right hand side
    constexpr ProdExprToken rhs() const noexcept { return _rhs; }

    /// @brief Returns the operation applied on the expression
    /// @return Any binary op that is not an assignment operator
    constexpr BinaryOp op() const noexcept { return static_cast<BinaryOp>(ExprBase::padding0); }
  };

  /// @brief Cast from built-in type to built-in type
  class CastExpr
    final : public ExprBase
  {
    /// @brief The expression to cast
    ProdExprToken _to_cast;

  public:
    /// @brief Constuctor
    /// @param range The range of tokens
    /// @param type The type to cast to
    /// @param to_cast The expression to cast
    /// @param is_bit_cast True if bit cast
    constexpr CastExpr(TokenRange range, TypeToken cast_to, ProdExprToken to_cast, bool is_bit_cast) noexcept
      : ExprBase(TypeToExprID<CastExpr>(), cast_to, range, is_bit_cast), _to_cast(to_cast) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(CastExpr);

    /// @brief Returns the expression to cast
    /// @return The expression to cast
    constexpr ProdExprToken to_cast() const noexcept { return _to_cast; }

    /// @brief Returns the type to cast
    /// @return The type to cast to
    constexpr TypeToken type_to_cast_to() const noexcept { return type(); }

    /// @brief Check if the cast is a bit cast
    /// @return True if bit cast
    constexpr bool is_bit_cast() const noexcept { return ExprBase::padding0; }
  };

  /// @brief Returns the address of a variable
  class AddressOfExpr
    final : public ExprBase
  {
    /// @brief The variable declaration whose address to return
    StmtExprToken _name;

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The type of the expression
    /// @param name The declaration of the variable whose address to return
    constexpr AddressOfExpr(TokenRange range, TypeToken type, StmtExprToken name) noexcept
      : ExprBase(TypeToExprID<AddressOfExpr>(), type, range), _name(name) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(AddressOfExpr);

    /// @brief Returns the declaration of the variable whose address to return
    /// @return The declaration of the variable whose address to return
    constexpr StmtExprToken name() const noexcept { return _name; }
  };

  /// @brief Represents a load from a pointer
  class PtrLoadExpr
    final : public ExprBase
  {
    /// @brief The expression whose result to load from
    ProdExprToken _to_load;

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The resulting type (must be the type pointed by 'load_from')
    /// @param load_from The expression whose result to load from
    constexpr PtrLoadExpr(TokenRange range, TypeToken type, ProdExprToken load_from) noexcept
      : ExprBase(TypeToExprID<PtrLoadExpr>(), type, range), _to_load(load_from) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(PtrLoadExpr);

    /// @brief Returns the expression pointer from which to load
    /// @return The expression whose result to load from
    constexpr ProdExprToken to_load() const noexcept { return _to_load; }
  };

  class ReadExpr
    : public ExprBase
  {
    /// @brief The declaration of the variable from which to read
    StmtExprToken _decl;

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The result of the read
    /// @param decl The variable declaration from which to read
    /// @param expr The ID of the expression
    constexpr ReadExpr(TokenRange range, TypeToken type, StmtExprToken decl, ExprID expr) noexcept
      : ExprBase(expr, type, range), _decl(decl) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(ReadExpr);

    /// @brief Returns the declaration from which to read.
    /// The returned StmtExprToken always represents a VarDeclExpr.
    /// @return The declaration from which to read
    constexpr StmtExprToken decl() const noexcept { return _decl; }    
  };

  /// @brief Represents a local variable read
  class VarReadExpr
    final : public ReadExpr
  {   
  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The result of the read
    /// @param decl The variable declaration from which to read
    constexpr VarReadExpr(TokenRange range, TypeToken type, StmtExprToken decl) noexcept
      : ReadExpr(range, type, decl, TypeToExprID<VarReadExpr>()) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(VarReadExpr);
  };

  /// @brief Represents a global variable read
  class GlobalReadExpr
    final : public ReadExpr
  {
  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The result of the read
    /// @param decl The variable declaration from which to read
    constexpr GlobalReadExpr(TokenRange range, TypeToken type, StmtExprToken decl) noexcept
      : ReadExpr(range, type, decl, TypeToExprID<GlobalReadExpr>()) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(GlobalReadExpr);
  };

  // TODO: add FnCallToken type.
  using FnCallToken = u32;

  /// @brief Represents a function call
  class FnCallExpr
    final : public ExprBase
  {
    /// @brief Contains all the informations about the function call
    FnCallToken payload;

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The return type of the function (can be void)
    /// @param call The function call informations
    constexpr FnCallExpr(TokenRange range, TypeToken type, FnCallToken call) noexcept
      : ExprBase(TypeToExprID<FnCallExpr>(), type, range), payload(call) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(FnCallExpr);
  };

  /// @brief Represents a write to a local variable
  class VarWriteExpr
    final : public ExprBase
  {
    /// @brief The declaration of the variable to which to write
    StmtExprToken _decl;
    /// @brief The value to write to the variable
    ProdExprToken value;

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The type (must be void)
    /// @param decl The declaration of the variable from which to write
    /// @param value The value to write to the variable
    constexpr VarWriteExpr(TokenRange range, TypeToken type, StmtExprToken decl, ProdExprToken value) noexcept
      : ExprBase(TypeToExprID<VarWriteExpr>(), type, range), _decl(decl), value(value) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(VarWriteExpr);

    /// @brief Returns the declaration of the variable from which to write
    /// @return The declaration of the variable from which to write
    constexpr StmtExprToken decl() const noexcept { return _decl; }

    /// @brief Returns the value to write to the variable
    /// @return The value to write to the variable
    constexpr ProdExprToken to_write() const noexcept { return value; }
  };

  /// @brief Represents a write to a global variable
  class GlobalWriteExpr
    final : public ExprBase
  {
    /// @brief The declaration of the variable to which to write
    StmtExprToken _decl;
    /// @brief The value to write to the variable
    ProdExprToken value;

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The type (must be void)
    /// @param decl The declaration of the variable from which to write
    /// @param value The value to write to the variable
    constexpr GlobalWriteExpr(TokenRange range, TypeToken type, StmtExprToken decl, ProdExprToken value) noexcept
      : ExprBase(TypeToExprID<GlobalWriteExpr>(), type, range), _decl(decl), value(value) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(GlobalWriteExpr);

    /// @brief Returns the declaration of the variable from which to write
    /// @return The declaration of the variable from which to write
    constexpr StmtExprToken decl() const noexcept { return _decl; }

    /// @brief Returns the value to write to the variable
    /// @return The value to write to the variable
    constexpr ProdExprToken to_write() const noexcept { return value; }
  };

  /// @brief Represents a store through a pointer
  class PtrStoreExpr
    final : public ExprBase
  {
    /// @brief The pointer in which to write
    ProdExprToken _where;
    /// @brief The value to write to store
    ProdExprToken _value;

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The type (must be void)
    /// @param where The pointer to which to write
    /// @param value The value to store to the pointer
    constexpr PtrStoreExpr(TokenRange range, TypeToken type, ProdExprToken where, ProdExprToken value) noexcept
      : ExprBase(TypeToExprID<PtrStoreExpr>(), type, range), _where(where), _value(value) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(PtrStoreExpr);

    /// @brief Returns the pointer to which to store
    /// @return The pointer to which to store
    constexpr ProdExprToken where() const noexcept { return _where; }

    /// @brief Returns the value to store
    /// @return The value to store
    constexpr ProdExprToken to_store() const noexcept { return _value; }
  };

  /// @brief Represents a move
  class MoveExpr
    final : public ExprBase
  {
    /// @brief The declaration from which to move
    StmtExprToken decl;
    /// @brief The declaration to which to move
    StmtExprToken to;

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The type (should be void)
    /// @param decl The declaration from which to move
    constexpr MoveExpr(TokenRange range, TypeToken type, StmtExprToken decl, StmtExprToken to) noexcept
      : ExprBase(TypeToExprID<MoveExpr>(), type, range), decl(decl), to(to) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(MoveExpr);

    /// @brief Returns the declaration of the variable from which to move
    /// @return The declaration of the variable from which to move
    constexpr StmtExprToken to_move() const noexcept { return decl; }
    /// @brief Returns the declaration of the variable to which to move
    /// @return The declaration of the variable to which to move
    constexpr StmtExprToken move_to() const noexcept { return to; }
  };
  
  /// @brief Represents a copy
  class CopyExpr
    final : public ExprBase
  {
    /// @brief The declaration from which to copy
    StmtExprToken decl;
    /// @brief The declaration to which to copy
    StmtExprToken to;

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The type (should be void)
    /// @param decl The declaration from which to copy
    constexpr CopyExpr(TokenRange range, TypeToken type, StmtExprToken decl, StmtExprToken to) noexcept
      : ExprBase(TypeToExprID<CopyExpr>(), type, range), decl(decl), to(to) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(CopyExpr);

    /// @brief Returns the declaration of the variable from which to copy
    /// @return The declaration of the variable from which to copy
    constexpr StmtExprToken to_copy() const noexcept { return decl; }
    /// @brief Returns the declaration of the variable to which to copy
    /// @return The declaration of the variable to which to copy
    constexpr StmtExprToken copy_to() const noexcept { return to; }
  };
  
  /// @brief Represents a conditional move
  class CMoveExpr
    final : public ExprBase
  {
    /// @brief The declaration from which to conditional move
    StmtExprToken decl;
    /// @brief The declaration to which to conditional move
    StmtExprToken to;

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The type (should be void)
    /// @param decl The declaration from which to conditional move
    constexpr CMoveExpr(TokenRange range, TypeToken type, StmtExprToken decl, StmtExprToken to) noexcept
      : ExprBase(TypeToExprID<CMoveExpr>(), type, range), decl(decl), to(to) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(CMoveExpr);

    /// @brief Returns the declaration of the variable from which to conditional move
    /// @return The declaration of the variable from which to conditional move
    constexpr StmtExprToken to_cmove() const noexcept { return decl; }
    /// @brief Returns the declaration of the variable to which to conditional move
    /// @return The declaration of the variable to which to conditional move
    constexpr StmtExprToken cmove_to() const noexcept { return to; }
  };

  /// @brief Represents a local variable declaration
  class VarDeclExpr
    final : public ExprBase
  {
    /// @brief The name of the variable
    StringView name;
    /// @brief The assigned value
    OptTok<ProdExprToken> value;
    /// @brief The local ID of the variable
    u32 _local_id;

  public:
    /// @brief Constructor, for initialized variables
    /// @param range The range of tokens
    /// @param type The type of the variable
    /// @param local_id The local ID of the variable
    /// @param name The name of the variable
    /// @param init The initial value of the variable
    /// @param is_mut True if the variable is mutable
    constexpr VarDeclExpr(TokenRange range, TypeToken type, u32 local_id, StringView name, OptTok<ProdExprToken> init, bool is_mut) noexcept
      : ExprBase(TypeToExprID<VarDeclExpr>(), type, range, is_mut), name(name), value(init), _local_id(local_id) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(VarDeclExpr);

    /// @brief Check if the variable was declared with an initial value.
    /// @return True if the variable was declared with an initial value
    constexpr bool is_init() const noexcept { return value.isValue(); }

    /// @brief Returns the initial value of the declared variable
    /// @return The initial value of the variable
    constexpr OptTok<ProdExprToken> init() const noexcept
    {
      return value;
    }

    /// @brief Returns the local ID of the variable.
    /// This is the "depth" in the spaghetti stack of variable declaration.
    /// @return The local ID of the variable
    constexpr u32 local_id() const noexcept { return _local_id; }

    /// @brief Check if the variable is mutable
    /// @return True if mutable
    constexpr bool is_mut() const noexcept { return padding0; }
    /// @brief Check if the variable is const
    /// @return True if not mutable
    constexpr bool is_const() const noexcept { return !is_mut(); }
  };
  
  /// @brief Represents a global variable
  class GlobalDeclExpr
    final : public ExprBase
  {
    /// @brief The name of the variable
    StringView name;
    /// @brief The assigned value
    ProdExprToken value;

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The type of the variable
    /// @param name The name of the variable
    /// @param init The initial value of the variable
    /// @param is_mut True if the variable is mutable
    constexpr GlobalDeclExpr(TokenRange range, TypeToken type, StringView name, ProdExprToken init, bool is_mut) noexcept
      : ExprBase(TypeToExprID<GlobalDeclExpr>(), type, range, is_mut), name(name), value(init) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(GlobalDeclExpr);

    /// @brief Returns the initial value of the declared variable
    /// @pre is_init()
    /// @return The initial value of the variable
    constexpr ProdExprToken init() const noexcept { return value; }

    /// @brief Check if the variable is mutable
    /// @return True if mutable
    constexpr bool is_mut() const noexcept { return padding0; }
    /// @brief Check if the variable is const
    /// @return True if not mutable
    constexpr bool is_const() const noexcept { return !is_mut(); }
  };

  /// @brief Represents a scope
  class ScopeExpr
    final : public ExprBase
  {
    /// @brief The parent
    OptTok<StmtExprToken> parent_expr;
    /// @brief The local variable declarations
    Vector<const VarDeclExpr*> decl{};
    /// @brief The statements contained in the scope (in the order of their declarations)
    Vector<ExprBase*> expressions{};

  public:
    /// @brief Constructs a scope with no parents.
    /// @param range The range of tokens
    /// @param type The type (must be void)
    constexpr ScopeExpr(TokenRange range, TypeToken type) noexcept
      : ExprBase(TypeToExprID<ScopeExpr>(), type, range), parent_expr(None) {}

    /// @brief Constructs a scope with a parent
    /// @param range The range of tokens
    /// @param type The type (must be void)
    /// @param parent The parent of the scope
    constexpr ScopeExpr(TokenRange range, TypeToken type, StmtExprToken parent) noexcept
      : ExprBase(TypeToExprID<ScopeExpr>(), type, range), parent_expr(parent) {}

    /// @brief Check if the current scope has a parent
    /// @return True if the scope has a parent
    constexpr bool has_parent() const noexcept { return parent_expr.isValue(); }

    /// @brief Returns the parent of the current scope.
    /// @return The parent of the current scope
    constexpr OptTok<StmtExprToken> parent() const noexcept
    {
      return parent_expr;
    }

    /// @brief Returns the local variable declarations of the current scope
    /// @return The declarations
    constexpr Vector<const VarDeclExpr*>& decls() noexcept { return decl; }
    /// @brief Returns the local variable declarations of the current scope
    /// @return The declarations
    constexpr const Vector<const VarDeclExpr*>& decls() const noexcept { return decl; }
    /// @brief Returns the expressions contained in the scope
    /// @return The expressions
    constexpr Vector<ExprBase*>& exprs() noexcept { return expressions; }
    /// @brief Returns the expressions contained in the scope
    /// @return The expressions
    constexpr const Vector<ExprBase*>& exprs() const noexcept { return expressions; }
  };

  /// @brief Represents a conditional expression
  class ConditionExpr
    final : public ExprBase
  {
    /// @brief The if condition
    ProdExprToken if_cond;
    /// @brief The if statement
    StmtExprToken if_stmt;
    /// @brief The else statement
    OptTok<StmtExprToken> else_stmt;

  public:
    /// @brief Constructs a condition expression that does not have an else branch
    /// @param range The range of tokens
    /// @param type The type (must be void)
    /// @param if_cond The if condition (must evaluate to bool)
    /// @param if_stmt The scope to execute if the condition evaluates to true
    /// @param else_stmt The scope to execute if the condition evaluates to false
    constexpr ConditionExpr(TokenRange range, TypeToken type, ProdExprToken if_cond, StmtExprToken if_stmt, OptTok<StmtExprToken> else_stmt)
      : ExprBase(TypeToExprID<ConditionExpr>(), type, range), if_cond(if_cond), if_stmt(if_stmt), else_stmt(else_stmt) {}
    
    /// @brief Check if this condition has an else branch
    /// @return True if this condition has an else branch
    constexpr bool has_else() const noexcept { return else_stmt.isValue(); }

    /// @brief Returns the else statement of the condition.
    /// @return The else statement
    constexpr OptTok<StmtExprToken> else_statement() const noexcept
    {
      return else_stmt;
    }
    
    /// @brief Returns the if statement of the condition
    /// @return The if statement
    constexpr StmtExprToken if_statement() const noexcept { return if_stmt; }

    /// @brief Returns the if condition of the condition
    /// @return The if condition
    constexpr ProdExprToken if_condition() const noexcept { return if_cond; }
  };

  template<typename T>
  struct producer_group_requirements
  {
    template<typename Ty>
    struct base_of
    {
      static constexpr bool value = std::is_base_of_v<T, Ty>;
    };

    using type = meta::type_list<COLTC_PROD_EXPR_LIST>::remove_if_not<base_of>;
  };
  
  template<typename T>
  using producer_group_requirements_t = typename producer_group_requirements<T>::type;
}

#endif // !HG_COLT_EXPR
