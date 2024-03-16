/*****************************************************************//**
 * @file   colt_expr.h
 * @brief  Contains the types used to represents the AST's nodes.
 * 
 * @author RPC
 * @date   March 2024
 *********************************************************************/
#ifndef HG_COLT_EXPR
#define HG_COLT_EXPR

#include "lex/colt_token_buffer.h"
#include "lng/colt_type_buffer.h"

DECLARE_ENUM_WITH_TYPE(u8, clt::lng, ExprID,
  EXPR_ERROR, EXPR_LITERAL, EXPR_UNARY, EXPR_BINARY,
  // VARIABLE RELATED
  EXPR_ADDRESSOF,
  EXPR_VAR_DECL, EXPR_VAR_READ, EXPR_VAR_WRITE,
  EXPR_GLOBAL_DECL, EXPR_GLOBAL_READ, EXPR_GLOBAL_WRITE,
  EXPR_MOVE, EXPR_COPY, EXPR_CMOVE,
  // PTR RELATED
  EXPR_PTR_LOAD, EXPR_PTR_STORE,
  // FN RELATED
  EXPR_DECL_FN,
  // OTHER
  EXPR_SCOPE, EXPR_CONDITION
);

/// @brief Macro Expr List (with same index as ExprID declaration!)
#define COLTC_EXPR_LIST ErrorExpr, LiteralExpr, UnaryExpr, BinaryExpr, \
   AddressOfExpr, VarDeclExpr, VarReadExpr, VarWriteExpr, \
   GlobalDeclExpr, GlobalReadExpr, GlobalWriteExpr, \
   MoveExpr, CopyExpr, CMoveExpr, \
   PtrLoadExpr, PtrStoreExpr, \
   FnDeclExpr, \
   ScopeExpr, ConditionExpr

namespace clt::lng
{
  // Forward declarations
  FORWARD_DECLARE_TYPE_LIST(COLTC_EXPR_LIST);
  // TypeToExprID
  CONVERT_TYPE_TO_ENUM(ExprID, COLTC_EXPR_LIST);

  using ExprToken = u32;

  /// @brief Base class of all expressions
  class ExprBase
  {
    /// @brief Represents the type of the expression
    TypeToken type;
    /// @brief Represents the range of Token forming the expression
    TokenRange range;
    /// @brief The ID of the expression
    ExprID expr_id;

  public:
    /// @brief Constructor
    /// @param id The ID of the expression
    /// @param type The type of the expression
    /// @param range The range representing the expression
    constexpr ExprBase(ExprID id, TypeToken type, TokenRange range) noexcept
      : type(type), range(range), expr_id(id) {}

    constexpr ExprBase(ExprBase&&) noexcept = default;
    constexpr ExprBase(const ExprBase&) noexcept = default;
    constexpr ExprBase& operator=(ExprBase&&) noexcept = default;
    constexpr ExprBase& operator=(const ExprBase&) noexcept = default;

    /// @brief Returns the range of tokens representing the expression
    /// @return Range of tokens reprsenting the expression
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

    constexpr ErrorExpr(ErrorExpr&&) noexcept = default;
    constexpr ErrorExpr(const ErrorExpr&) noexcept = default;
    constexpr ErrorExpr& operator=(ErrorExpr&&) noexcept = default;
    constexpr ErrorExpr& operator=(const ErrorExpr&) noexcept = default;
  };

  class ExprBuffer
  {

  };
}

#endif // !HG_COLT_EXPR
