/*****************************************************************/ /**
 * @file   colt_expr_token.h
 * @brief  Contains the tokens that represent expressions.
 *
 * @author RPC
 * @date   March 2024
 *********************************************************************/
#ifndef HG_COLT_EXPR_TOKEN
#define HG_COLT_EXPR_TOKEN

#include "meta/meta_type_list.h"
#include "lng/union_macro.h"
#include "lex/colt_token_buffer.h"
#include "lng/colt_type_buffer.h"

DECLARE_ENUM_WITH_TYPE(
    u8, clt::lng, ExprID, EXPR_ERROR, EXPR_NOP, EXPR_LITERAL, EXPR_UNARY,
    EXPR_BINARY, EXPR_CAST,
    // VARIABLE RELATED
    EXPR_ADDRESSOF, EXPR_VAR_DECL, EXPR_VAR_READ, EXPR_VAR_WRITE, EXPR_GLOBAL_DECL,
    EXPR_GLOBAL_READ, EXPR_GLOBAL_WRITE, EXPR_MOVE, EXPR_COPY, EXPR_CMOVE,
    // PTR RELATED
    EXPR_PTR_LOAD, EXPR_PTR_STORE,
    // FN RELATED
    EXPR_CALL_FN,
    // OTHER
    EXPR_SCOPE, EXPR_CONDITION);

/// @brief Macro Expr List (with same index as ExprID declaration!)
#define COLTC_EXPR_LIST                                                            \
  ErrorExpr, NOPExpr, LiteralExpr, UnaryExpr, BinaryExpr, CastExpr, AddressOfExpr, \
      VarDeclExpr, VarReadExpr, VarWriteExpr, GlobalDeclExpr, GlobalReadExpr,      \
      GlobalWriteExpr, MoveExpr, CopyExpr, CMoveExpr, PtrLoadExpr, PtrStoreExpr,   \
      FnCallExpr, ScopeExpr, ConditionExpr

#define COLTC_PROD_EXPR_LIST                                                       \
  ErrorExpr, NOPExpr, LiteralExpr, UnaryExpr, BinaryExpr, CastExpr, AddressOfExpr, \
      PtrLoadExpr, VarReadExpr, GlobalReadExpr, FnCallExpr, VarWriteExpr,          \
      PtrStoreExpr, GlobalWriteExpr, MoveExpr, CopyExpr, CMoveExpr

#define COLTC_STMT_EXPR_LIST \
  VarDeclExpr, GlobalDeclExpr, ScopeExpr, ConditionExpr, ErrorExpr

namespace clt::lng
{
  // Forward declarations
  FORWARD_DECLARE_TYPE_LIST(COLTC_EXPR_LIST);
  // TypeToExprID
  CONVERT_TYPES_TO_ENUM(ExprID, COLTC_EXPR_LIST);

  // Forward declarations
  class ExprBuffer;
  class ExprBase;

  /// @brief Represents any expression that produces a value (or writes).
  /// Can be any of [COLTC_PROD_EXPR_LIST]
  CREATE_TOKEN_TYPE(
      ProdExprToken, u32, std::numeric_limits<u32>::max() - 1, ExprBuffer, ExprBase);

  /// @brief Represents a statement.
  /// Can be any of [COLTC_STMT_EXPR_LIST]
  CREATE_TOKEN_TYPE(
      StmtExprToken, u32, std::numeric_limits<u32>::max() - 1, ExprBuffer, ExprBase);
} // namespace clt::lng

#endif //HG_COLT_EXPR_TOKEN