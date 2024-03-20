/*****************************************************************//**
 * @file   colt_expr.h
 * @brief  Contains the types used to represents the AST's nodes.
 * 
 * @author RPC
 * @date   March 2024
 *********************************************************************/
#ifndef HG_COLT_EXPR
#define HG_COLT_EXPR

#include "meta/meta_type_list.h"
#include "lng/macro_helper.h"
#include "lex/colt_token_buffer.h"
#include "lng/colt_type_buffer.h"

DECLARE_ENUM_WITH_TYPE(u8, clt::lng, ExprID,
  EXPR_ERROR, EXPR_LITERAL, EXPR_UNARY, EXPR_BINARY, EXPR_CAST,
  // VARIABLE RELATED
  EXPR_ADDRESSOF,
  EXPR_VAR_DECL, EXPR_VAR_READ, EXPR_VAR_WRITE,
  EXPR_GLOBAL_DECL, EXPR_GLOBAL_READ, EXPR_GLOBAL_WRITE,
  EXPR_MOVE, EXPR_COPY, EXPR_CMOVE,
  // PTR RELATED
  EXPR_PTR_LOAD, EXPR_PTR_STORE,
  // FN RELATED
  EXPR_DECL_FN, EXPR_CALL_FN,
  // OTHER
  EXPR_SCOPE, EXPR_CONDITION
);

/// @brief Macro Expr List (with same index as ExprID declaration!)
#define COLTC_EXPR_LIST ErrorExpr, LiteralExpr, UnaryExpr, BinaryExpr, CastExpr, \
   AddressOfExpr, VarDeclExpr, VarReadExpr, VarWriteExpr, \
   GlobalDeclExpr, GlobalReadExpr, GlobalWriteExpr, \
   MoveExpr, CopyExpr, CMoveExpr, \
   PtrLoadExpr, PtrStoreExpr, \
   FnDeclExpr, FnCallExpr, \
   ScopeExpr, ConditionExpr

#define COLTC_PROD_EXPR_LIST ErrorExpr, LiteralExpr, UnaryExpr, \
  BinaryExpr, CastExpr, AddressOfExpr, PtrLoadExpr, VarReadExpr, GlobalReadExpr, FnCallExpr

#define COLTC_SINK_EXPR_LIST ErrorExpr, VarWriteExpr, \
  PtrStoreExpr, GlobalWriteExpr, MoveExpr, CopyExpr, CMoveExpr

#define COLTC_STMT_EXPR_LIST ErrorExpr, VarDeclExpr, GlobalDeclExpr, ScopeExpr, ConditionExpr

namespace clt::lng
{
  // Forward declarations
  FORWARD_DECLARE_TYPE_LIST(COLTC_EXPR_LIST);
  // TypeToExprID
  CONVERT_TYPES_TO_ENUM(ExprID, COLTC_EXPR_LIST);

  /// @brief Represents any expression that produces a value.
  /// Can be any of [ErrorExpr, LiteralExpr, UnaryExpr, BinaryExpr, CastExpr,
  /// AddressOfExpr, PtrLoadExpr, VarReadExpr, GlobalReadExpr, FnCallExpr]
  using ProdExprToken = u32;

  template<typename T>
  concept ProducerExpr = meta::is_any_of<T, COLTC_PROD_EXPR_LIST>;

  /// @brief Represents any of [ErrorExpr, VarWriteExpr,
  /// PtrStoreExpr, GlobalWriteExpr, MoveExpr, CopyExpr, CMoveExpr]
  using SinkExprToken = u32;

  template<typename T>
  concept SinkExpr = meta::is_any_of<T, COLTC_SINK_EXPR_LIST>;

  /// @brief Represents any of [ErrorExpr, VarDeclExpr, GlobalDeclExpr, ScopeExpr, ConditionExpr]
  using StmtExprToken = u32;

  template<typename T>
  concept StatementExpr = meta::is_any_of<T, COLTC_STMT_EXPR_LIST>;

  /// @brief Base class of all expressions
  class ExprBase
  {
    /// @brief Represents the type of the expression
    TypeToken type;
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
      : type(type), range(range), expr_id(id), padding0(padding0), padding1(padding1), padding2(padding2) {}

    ExprBase() = delete;
    MAKE_DEFAULT_COPY_AND_MOVE_FOR(ExprBase);

    /// @brief Returns the range of tokens representing the expression
    /// @return Range of tokens representing the expression
    constexpr TokenRange getTokenRange() const noexcept { return range; }
    /// @brief Returns the type of the expression.
    /// @return The type of the expression
    constexpr TypeToken getType() const noexcept { return type; }

    /// @brief Returns the expression ID
    /// @return The expression ID
    constexpr ExprID classof() const noexcept { return expr_id; }

    /// @brief Check if the current expression represents an ErrorExpr
    /// @return True if ErrorExpr
    constexpr bool isError() const noexcept { return classof() == ExprID::EXPR_ERROR; }
  };

  /// @brief Represents an error expression.
  /// Used to avoid reporting a lot of errors.
  class ErrorExpr
    final : public ExprBase
  {
    /// @brief Constructor
    /// @param id The ID of the expression
    /// @param type The type of the expression
    /// @param range The range representing the expression
    constexpr ErrorExpr(TokenRange range, TypeToken type) noexcept
      : ExprBase(TypeToExprID<ErrorExpr>(), type, range) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(ErrorExpr);
  };

  /// @brief Represents a literal expression
  class LiteralExpr
    final : public ExprBase
  {
    /// @brief The value of the literal
    QWORD_t value;

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The type of the literal
    /// @param value The literal value
    constexpr LiteralExpr(TokenRange range, TypeToken type, QWORD_t value) noexcept
      : ExprBase(TypeToExprID<LiteralExpr>(), type, range), value(value) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(LiteralExpr);

    /// @brief Return the value of the literal
    /// @return The value of the literal
    constexpr QWORD_t getValue() const noexcept { return value; }
  };

  /// @brief Represents a unary expression
  class UnaryExpr
    final : public ExprBase
  {
    /// @brief The expression on which the operation is applied
    ProdExprToken expr;

    // The operator is stored in the padding bits of ExprBase.

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The type of the expression (same as 'expr')
    /// @param op The operation
    /// @param expr The expression on which the unary operation is applied
    constexpr UnaryExpr(TokenRange range, TypeToken type, UnaryOp op, ProdExprToken expr) noexcept
      : ExprBase(TypeToExprID<UnaryExpr>(), type, range, static_cast<u8>(op)), expr(expr) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(UnaryExpr);

    /// @brief Returns the expression on which the unary operation is applied.
    /// @return The expression on which the unary operation is applied
    constexpr ProdExprToken getExpr() const noexcept { return expr; }

    /// @brief Returns the operation applied on the expression
    /// @return Any unary op
    constexpr UnaryOp getOp() const noexcept { return static_cast<UnaryOp>(ExprBase::padding0); }
  };

  /// @brief Reprents a binary operation
  class BinaryExpr
    final : public ExprBase
  {
    /// @brief The left hand side of the expression
    ProdExprToken lhs;
    /// @brief The right hand side of the expression
    ProdExprToken rhs;

    // The operator is stored in the padding bits of ExprBase.

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The type of the expression (can differ if comparison)
    /// @param lhs The left hand side of the expression
    /// @param op The operation applied on the expression
    /// @param rhs The right hand side of the expression
    constexpr BinaryExpr(TokenRange range, TypeToken type, ProdExprToken lhs, BinaryOp op, ProdExprToken rhs) noexcept
      : ExprBase(TypeToExprID<UnaryExpr>(), type, range, static_cast<u8>(op)), lhs(lhs), rhs(rhs) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(BinaryExpr);

    /// @brief Returns the left hand side of the expression
    /// @return The left hand side
    constexpr ProdExprToken getLHS() const noexcept { return lhs; }
    /// @brief Returns the right hand side of the expression
    /// @return The right hand side
    constexpr ProdExprToken getRHS() const noexcept { return rhs; }

    /// @brief Returns the operation applied on the expression
    /// @return Any binary op that is not an assignment operator
    constexpr BinaryOp getOp() const noexcept { return static_cast<BinaryOp>(ExprBase::padding0); }
  };

  /// @brief Cast from built-in type to built-in type
  class CastExpr
    final : public ExprBase
  {
    /// @brief The expression to cast
    ProdExprToken to_cast;

  public:
    /// @brief Constuctor
    /// @param range The range of tokens
    /// @param type The type to cast to
    /// @param to_cast The expression to cast
    /// @param is_bit_cast True if bit cast
    constexpr CastExpr(TokenRange range, TypeToken cast_to, ProdExprToken to_cast, bool is_bit_cast) noexcept
      : ExprBase(TypeToExprID<CastExpr>(), cast_to, range, is_bit_cast), to_cast(to_cast) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(CastExpr);

    /// @brief Returns the expression to cast
    /// @return The expression to cast
    constexpr ProdExprToken getToCast() const noexcept { return to_cast; }

    /// @brief Returns the type to cast
    /// @return The type to cast to
    constexpr TypeToken getTypeToCastTo() const noexcept { return getType(); }

    /// @brief Check if the cast is a bit cast
    /// @return True if bit cast
    constexpr bool isBitCast() const noexcept { return ExprBase::padding0; }
  };

  /// @brief Returns the address of a variable
  class AddressOfExpr
    final : public ExprBase
  {
    /// @brief The variable declaration whose address to return
    StmtExprToken name;

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The type of the expression
    /// @param name The declaration of the variable whose address to return
    constexpr AddressOfExpr(TokenRange range, TypeToken type, StmtExprToken name) noexcept
      : ExprBase(TypeToExprID<AddressOfExpr>(), type, range), name(name) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(AddressOfExpr);

    /// @brief Returns the declaration of the variable whose address to return
    /// @return The declaration of the variable whose address to return
    constexpr StmtExprToken getName() const noexcept { return name; }
  };

  /// @brief Represents a load from a pointer
  class PtrLoadExpr
    final : public ExprBase
  {
    /// @brief The expression whose result to load from
    ProdExprToken to_load;

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The resulting type (must be the type pointed by 'load_from')
    /// @param load_from The expression whose result to load from
    constexpr PtrLoadExpr(TokenRange range, TypeToken type, ProdExprToken load_from) noexcept
      : ExprBase(TypeToExprID<PtrLoadExpr>(), type, range), to_load(load_from) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(PtrLoadExpr);

    /// @brief Returns the expression pointer from which to load
    /// @return The expression whose result to load from
    constexpr ProdExprToken getToLoad() const noexcept { return to_load; }
  };

  /// @brief Represents a local variable read
  class VarReadExpr
    final : public ExprBase
  {
    /// @brief The declaration of the variable from which to read
    StmtExprToken decl;

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The result of the read
    /// @param decl The variable declaration from which to read
    constexpr VarReadExpr(TokenRange range, TypeToken type, StmtExprToken decl) noexcept
      : ExprBase(TypeToExprID<VarReadExpr>(), type, range), decl(decl) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(VarReadExpr);

    /// @brief Returns the declaration from which to read.
    /// The returned StmtExprToken always represents a VarDeclExpr.
    /// @return The declaration from which to read
    constexpr StmtExprToken getDecl() const noexcept { return decl; }
  };

  /// @brief Represents a global variable read
  class GlobalReadExpr
    final : public ExprBase
  {
    /// @brief The declaration of the variable from which to read
    StmtExprToken decl;

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The result of the read
    /// @param decl The variable declaration from which to read
    constexpr GlobalReadExpr(TokenRange range, TypeToken type, StmtExprToken decl) noexcept
      : ExprBase(TypeToExprID<VarReadExpr>(), type, range), decl(decl) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(GlobalReadExpr);

    /// @brief Returns the declaration from which to read.
    /// The returned StmtExprToken always represents a GlobalDeclExpr.
    /// @return The declaration from which to read
    constexpr StmtExprToken getDecl() const noexcept { return decl; }
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
    StmtExprToken decl;
    /// @brief The value to write to the variable
    ProdExprToken value;

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The type (must be void)
    /// @param decl The declaration of the variable from which to write
    /// @param value The value to write to the variable
    constexpr VarWriteExpr(TokenRange range, TypeToken type, StmtExprToken decl, ProdExprToken value) noexcept
      : ExprBase(TypeToExprID<VarWriteExpr>(), type, range), decl(decl), value(value) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(VarWriteExpr);

    /// @brief Returns the declaration of the variable from which to write
    /// @return The declaration of the variable from which to write
    constexpr StmtExprToken getDecl() const noexcept { return decl; }

    /// @brief Returns the value to write to the variable
    /// @return The value to write to the variable
    constexpr ProdExprToken getToWrite() const noexcept { return value; }
  };

  /// @brief Represents a write to a global variable
  class GlobalWriteExpr
    final : public ExprBase
  {
    /// @brief The declaration of the variable to which to write
    StmtExprToken decl;
    /// @brief The value to write to the variable
    ProdExprToken value;

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The type (must be void)
    /// @param decl The declaration of the variable from which to write
    /// @param value The value to write to the variable
    constexpr GlobalWriteExpr(TokenRange range, TypeToken type, StmtExprToken decl, ProdExprToken value) noexcept
      : ExprBase(TypeToExprID<GlobalWriteExpr>(), type, range), decl(decl), value(value) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(GlobalWriteExpr);

    /// @brief Returns the declaration of the variable from which to write
    /// @return The declaration of the variable from which to write
    constexpr StmtExprToken getDecl() const noexcept { return decl; }

    /// @brief Returns the value to write to the variable
    /// @return The value to write to the variable
    constexpr ProdExprToken getToWrite() const noexcept { return value; }
  };

  /// @brief Represents a store through a pointer
  class PtrStoreExpr
    final : public ExprBase
  {
    /// @brief The pointer in which to write
    ProdExprToken where;
    /// @brief The value to write to store
    ProdExprToken value;

  public:
    /// @brief Constructor
    /// @param range The range of tokens
    /// @param type The type (must be void)
    /// @param where The pointer to which to write
    /// @param value The value to store to the pointer
    constexpr PtrStoreExpr(TokenRange range, TypeToken type, ProdExprToken where, ProdExprToken value) noexcept
      : ExprBase(TypeToExprID<PtrStoreExpr>(), type, range), where(where), value(value) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(PtrStoreExpr);

    /// @brief Returns the pointer to which to store
    /// @return The pointer to which to store
    constexpr ProdExprToken getWhere() const noexcept { return where; }

    /// @brief Returns the value to store
    /// @return The value to store
    constexpr ProdExprToken getToStore() const noexcept { return value; }
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
    /// @param type The type (should be move)
    /// @param decl The declaration from which to move
    constexpr MoveExpr(TokenRange range, TypeToken type, StmtExprToken decl, StmtExprToken to) noexcept
      : ExprBase(TypeToExprID<MoveExpr>(), type, range), decl(decl), to(to) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(MoveExpr);

    /// @brief Returns the declaration of the variable from which to move
    /// @return The declaration of the variable from which to move
    constexpr StmtExprToken getToMove() const noexcept { return decl; }
    /// @brief Returns the declaration of the variable to which to move
    /// @return The declaration of the variable to which to move
    constexpr StmtExprToken getMoveTo() const noexcept { return to; }
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
    /// @param type The type (should be copy)
    /// @param decl The declaration from which to copy
    constexpr CopyExpr(TokenRange range, TypeToken type, StmtExprToken decl, StmtExprToken to) noexcept
      : ExprBase(TypeToExprID<CopyExpr>(), type, range), decl(decl), to(to) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(CopyExpr);

    /// @brief Returns the declaration of the variable from which to copy
    /// @return The declaration of the variable from which to copy
    constexpr StmtExprToken getToCopy() const noexcept { return decl; }
    /// @brief Returns the declaration of the variable to which to copy
    /// @return The declaration of the variable to which to copy
    constexpr StmtExprToken getCopyTo() const noexcept { return to; }
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
    /// @param type The type (should be conditional move)
    /// @param decl The declaration from which to conditional move
    constexpr CMoveExpr(TokenRange range, TypeToken type, StmtExprToken decl, StmtExprToken to) noexcept
      : ExprBase(TypeToExprID<CMoveExpr>(), type, range), decl(decl), to(to) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(CMoveExpr);

    /// @brief Returns the declaration of the variable from which to conditional move
    /// @return The declaration of the variable from which to conditional move
    constexpr StmtExprToken getToCMove() const noexcept { return decl; }
    /// @brief Returns the declaration of the variable to which to conditional move
    /// @return The declaration of the variable to which to conditional move
    constexpr StmtExprToken getCMoveTo() const noexcept { return to; }
  };
}

#endif // !HG_COLT_EXPR
