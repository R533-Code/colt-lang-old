/*****************************************************************//**
 * @file   ast.cpp
 * @brief  Contains the implementation of 'ast.h'.
 *
 * @author RPC
 * @date   April 2024
 *********************************************************************/
#include "ast.h"

 /// @brief Pops the elements added to a vector in the current scope at the end of the scope
#define SCOPED_SAVE_VECTOR(vec) const auto COLT_CONCAT(SCOPED_SIZE_, COLT_LINE_NUM) = vec.size(); ON_SCOPE_EXIT { vec.pop_back_n(vec.size() - COLT_CONCAT(SCOPED_SIZE_, COLT_LINE_NUM)); }

#define __IMPL_WRAP_IN_IILAMBDA(expr) [&]() { return expr; }()
#define PROPAGATE_ERROR(expr, to_ret) \
[&](auto f) {\
using ___IMPL_type = std::remove_cvref_t<decltype(expr)>; \
if constexpr (std::same_as<___IMPL_type, clt::ErrorFlag>) \
  if (__IMPL_WRAP_IN_IILAMBDA(expr.is_error())) \
    return to_ret; \
if constexpr (std::same_as<___IMPL_type, clt::lng::StmtExprToken>) \
  if (__IMPL_WRAP_IN_IILAMBDA(Expr(expr).isError())) \
    return to_ret; \
if constexpr (std::same_as<___IMPL_type, clt::lng::StmtExprVariant>) \
  if (__IMPL_WRAP_IN_IILAMBDA(expr.isError())) \
    return to_ret; \
if constexpr (std::same_as<___IMPL_type, clt::lng::ProdExprToken>) \
  if (__IMPL_WRAP_IN_IILAMBDA(Expr(expr).isError())) \
    return to_ret; \
if constexpr (std::same_as<___IMPL_type, clt::lng::ProdExprVariant>) \
  if (__IMPL_WRAP_IN_IILAMBDA(expr.isError())) \
    return to_ret; \
if constexpr (std::same_as<___IMPL_type, clt::lng::TypeToken>) \
  if (__IMPL_WRAP_IN_IILAMBDA(Type(expr).isError())) \
    return to_ret; \
if constexpr (std::same_as<___IMPL_type, clt::lng::TypeVariant>) \
  if (__IMPL_WRAP_IN_IILAMBDA(expr.isError())) \
    return to_ret; \
clt::unreachable("Invalid type!"); \
}([&]() -> decltype(auto) {})


namespace clt::lng
{
  void MakeAST(ParsedUnit& unit) noexcept
  {
    assert_true("Unit already parsed!", !unit.isParsed());
    // The constructor generates the AST directly
    ASTMaker ast = { unit };
  }

  ProdExprToken ASTMaker::parse_primary_literal(const TokenRangeGenerator& range) noexcept
  {
    assert_true("Expected literal token!", isLiteralToken(current()));
    //Save literal token
    auto literal_tkn = current();

    //Consume the literal token
    consume_current();

    //TODO: add support for string literal
    QWORD_t value = getTokenBuffer().getLiteral(literal_tkn);
    return Expr().addLiteral(
      range.getRange(), value, LiteralToBuiltinID(literal_tkn)
    );
  }

  ProdExprToken ASTMaker::parse_primary_invalid(const TokenRangeGenerator& range) noexcept
  {
    using enum Lexeme;

    // The error would have been generated by the lexer
    if (current() == TKN_ERROR)
      consume_current();
    else
      report_current<ERROR>(current_panic, "Expected an expression!");

    return Expr().addError(range.getRange());
  }

  bool ASTMaker::warnFor(run::OpError err) const noexcept
  {
    assert_true("DIV_BY_ZERO is an error not a warning!", err != run::OpError::DIV_BY_ZERO);
    switch_no_default(err)
    {
      case run::OpError::RET_NAN:
      case run::OpError::WAS_NAN:
        return getWarnFor().constant_folding_nan;
      case run::OpError::SIGNED_OVERFLOW:
      case run::OpError::SIGNED_UNDERFLOW:
        return getWarnFor().constant_folding_signed_ou;
      case run::OpError::UNSIGNED_OVERFLOW:
      case run::OpError::UNSIGNED_UNDERFLOW:
        return getWarnFor().constant_folding_unsigned_ou;
      case run::OpError::SHIFT_BY_GRE_SIZEOF:
        return getWarnFor().constant_folding_invalid_shift;
      case run::OpError::NO_ERROR:
        return false;
    }
  }

  void ASTMaker::handle_comparison_chain_error(Token comparison, ComparisonSet comparison_set) noexcept
  {
    using enum ComparisonSet;
    
    if (getComparisonSet(comparison) == NONE)
      report<report_as::ERROR>(comparison, nullptr,
        "'{}' cannot be chained with any other comparison operators!", toStr(TokenToBinary(comparison)));
    else
      report<report_as::ERROR>(comparison, nullptr,
        "'{}' cannot be chained with {}!", toStr(TokenToBinary(comparison)), toStr(comparison_set));
  }

  ProdExprToken ASTMaker::parse_primary(bool accepts_conv)
  {
    using enum Lexeme;
    auto depth = addDepth();
    auto range = startRange();

    uninit<ProdExprToken> to_ret;

    // Handles literals (10, 0.5, ...)
    if (isLiteralToken(current()))
      to_ret = parse_primary_literal(range);
    // Handles unary operators (&, *, -, ...)
    else if (isUnaryToken(current()))
      to_ret = parse_unary();
    // Handles (...)
    else if (current() == TKN_LEFT_PAREN)
      to_ret = parse_parenthesis(&ASTMaker::parse_binary);
    // Handles invalid primary expressions
    else
      to_ret = parse_primary_invalid(range);

    //If the primary expression accepts a conversion, check for as/bit_as.
    if (accepts_conv && is_current_one_of(TKN_KEYWORD_bit_as, TKN_KEYWORD_as))
      return parse_conversion(to_ret, range);
    return to_ret;
  }

  ProdExprToken ASTMaker::parse_unary_and(ProdExprToken child, const TokenRangeGenerator& range) noexcept
  {
    if (auto pchild = decl_from_read(child); pchild.isValue())
      return Expr().addAddressOf(range.getRange(), pchild.getValue());

    report<ERROR>(range.getRange(), nullptr,
      "Unary '&' can only be applied on a variable!");
    return Expr().addError(range.getRange());
  }

  ProdExprToken ASTMaker::parse_unary_star(ProdExprToken child, const TokenRangeGenerator& range) noexcept
  {
    if (!Type(child).isAnyOpaquePtr())
    {
      report<ERROR>(range.getRange(), nullptr,
        "Unary '*' can only be applied on a non-opaque pointer!");
      return Expr().addError(range.getRange());
    }
    else if (!Type(child).isAnyPtr())
    {
      report<ERROR>(range.getRange(), nullptr,
        "Unary '*' can only be applied on pointer types!");
      return Expr().addError(range.getRange());
    }
    return Expr().addPtrLoad(range.getRange(), child);
  }

  ProdExprToken ASTMaker::parse_unary()
  {
    assert_true("parse_unary must be called when a unary token is hit!", isUnaryToken(current()));
    using enum Lexeme;
    auto depth = addDepth();
    auto range = startRange();

    uninit<ProdExprToken> to_ret;

    //Save the operator
    Token op = current();
    consume_current(); //consume the unary operator

    //Parse the child expression, without handling conversions:
    // '-5 as i32' is equivalent to '(-5) as i32'
    ProdExprToken child = parse_primary(false);
    PROPAGATE_ERROR(child, range.getRange());

    switch (op)
    {
    case TKN_PLUS_PLUS:
    case TKN_MINUS_MINUS:

      // Handles '+', which are not supported by the language
    break; case TKN_PLUS:
      report<ERROR>(range.getRange(), current_panic, "Unary '+' is not supported!");
      to_ret = Expr().addError(range.getRange());

      // Handles '&', which are usually AddressOf expressions
    break; case TKN_AND:
      to_ret = parse_unary_and(child, range);

      // Handles '*', which are usually PtrLoad expressions
    break; case TKN_STAR:
      to_ret = parse_unary_star(child, range);

      // Handles the rest of the unary tokens (makeUnary checks for supports())
    break; default:
      to_ret = makeUnary(range.getRange(), TokenToUnary(op), child);
    }
    return to_ret;
  }

  ProdExprToken ASTMaker::parse_binary()
  {
    using enum Lexeme;
    auto depth = addDepth();
    auto range = startRange();

    ProdExprToken lhs = parse_primary();
    PROPAGATE_ERROR(lhs, lhs);

    // The binary operators
    Token binary_op = current();
    if (isAssignmentToken(binary_op))
      return parse_assignment(lhs, range);
    if (isComparisonToken(binary_op))
    {
      lhs = parse_comparison(binary_op, lhs, range);
      binary_op = current();
    }

    const u8 precedence = 0;
    //The current operator's precedence
    u8 op_precedence = OpPrecedence(binary_op);
    while (op_precedence > precedence)
    {
      //Consume the operator
      consume_current();
      //Recurse: 10 + 5 + 8 -> (10 + (5 + 8))
      ProdExprToken rhs = parse_binary_internal(binary_op);
      PROPAGATE_ERROR(rhs, rhs);

      if (!isBinaryToken(binary_op))
      {
        report<report_as::ERROR>(binary_op, current_panic, "Expected a binary operator!");
        return Expr().addError(range.getRange());
      }
      //Pratt's parsing, which allows operators priority
      lhs = makeBinary(range.getRange(), lhs, TokenToBinary(binary_op), rhs);
      if (isComparisonToken(current()))
        lhs = parse_comparison(current(), lhs, range);

      //Update the Token
      binary_op = current();
      //Update precedence
      op_precedence = OpPrecedence(binary_op);
    }

    return lhs;
  }

  ProdExprToken ASTMaker::parse_binary_condition()
  {
    auto condition = parse_binary();
    if (Expr(condition).isError())
      return condition;

    auto range = Expr(condition).getTokenRange();
    if (!Type(condition).isBuiltinAnd(&isBool))
    {
      report<report_as::ERROR>(range, nullptr,
        "Expression should be of type 'bool'!");
      return Expr().addError(range);
    }
    //If the expression is not a comparison, but is of type bool (read from
    // boolean variable, ...), transform it to a comparison with 'true'
    else if (!Expr(condition).is<BinaryExpr>())
    {
      QWORD_t true_value = 1;
      condition = makeBinary(range, condition, BinaryOp::OP_EQUAL,
        Expr().addLiteral(range, true_value, BuiltinID::BOOL));
    }
    return condition;
  }

  ProdExprToken ASTMaker::parse_binary_internal(Token previous)
  {
    using enum Lexeme;
    auto depth = addDepth();
    auto range = startRange();

    ProdExprToken lhs = parse_primary();
    PROPAGATE_ERROR(lhs, lhs);

    // The binary operators
    Token binary_op = current();
    //The current operator's precedence
    u8 op_precedence = OpPrecedence(binary_op);
    while (op_precedence > OpPrecedence(previous))
    {
      //Consume the operator
      consume_current();
      //Recurse: 10 + 5 + 8 -> (10 + (5 + 8))
      ProdExprToken rhs = parse_binary_internal(binary_op);
      PROPAGATE_ERROR(rhs, rhs);

      if (!isBinaryToken(binary_op))
      {
        report<report_as::ERROR>(binary_op, current_panic, "Expected a binary operator!");
        return Expr().addError(range.getRange());
      }
      else if (isComparisonToken(binary_op))
        lhs = parse_comparison(binary_op, lhs, rhs, range);
      else //Pratt's parsing, which allows operators priority
        lhs = makeBinary(range.getRange(), lhs, TokenToBinary(binary_op), rhs);

      //Update the Token
      binary_op = current();
      //Update precedence
      op_precedence = OpPrecedence(binary_op);
    }

    return lhs;
  }  

  ProdExprToken ASTMaker::parse_comparison(Token comparison, ProdExprToken lhs, const TokenRangeGenerator& range)
  {
    using enum Lexeme;
    using enum ComparisonSet;
    assert_true("parse_comparison must be called when isComparisonToken(current())", isComparisonToken(comparison));
    
    if (comparison == current())
      consume_current();    
    return parse_comparison(comparison, lhs, parse_binary_internal(comparison), range);
  }

  ProdExprToken ASTMaker::parse_comparison(Token comparison, ProdExprToken lhs, ProdExprToken rhs, const TokenRangeGenerator& range)
  {
    ComparisonSet comparison_set = getComparisonSet(comparison);
    auto ret = makeBinary(range.getRange(), lhs, TokenToBinary(comparison), rhs);

    while (isComparisonToken(current()))
    {
      comparison = current();
      if (isInvalidChain(comparison_set, getComparisonSet(comparison)))
        handle_comparison_chain_error(comparison, comparison_set);

      consume_current();
      auto nrhs = parse_binary_internal(comparison);
      ret = makeBinary(range.getRange(),
        ret, BinaryOp::OP_BOOL_AND,
        makeBinary(range.getRange(), rhs, TokenToBinary(comparison), nrhs));
      rhs = nrhs;
    }
    return ret;
  }

  ProdExprToken ASTMaker::parse_conversion(ProdExprToken to_conv, const TokenRangeGenerator& range)
  {
    using enum Lexeme;
    auto depth = addDepth();
    assert_true("Function should only be called when 'as' or 'bit_as' is encountered!",
      current() == TKN_KEYWORD_as || current() == TKN_KEYWORD_bit_as);

    auto cnv = current();
    consume_current();

    TypeToken cnv_type = parse_typename();
    PROPAGATE_ERROR(cnv_type, Expr().addError(range.getRange()));
    PROPAGATE_ERROR(to_conv, to_conv);

    // A bit_as conversion must have either the target or the starting
    // type be a byte type
    if (cnv == TKN_KEYWORD_bit_as
      && !(Type(cnv_type).isBuiltinAnd(&isBytes) || Type(to_conv).isBuiltinAnd(&isBytes)))
    {
      report<report_as::ERROR>(range.getRange(), nullptr,
        "'bit_as' conversion can only be applied on/to bytes types!");
      getReporter().message("Bytes types are 'BYTE', 'WORD', 'DWORD' and 'QWORD'.");
      return Expr().addError(range.getRange());
    }

    return makeCast(range.getRange(), to_conv, cnv_type, cnv == TKN_KEYWORD_bit_as);
  }

  ProdExprToken ASTMaker::parse_assignment(ProdExprToken assign_to, const TokenRangeGenerator& range)
  {
    auto panic = scopedSetPanic(&ASTMaker::panic_consume_semicolon);
    return assign_to;
  }

  StmtExprToken ASTMaker::parse_scope(bool accepts_single)
  {
    using enum Lexeme;
    auto depth = addDepth();
    auto range = startRange();

    SCOPED_SAVE_VECTOR(local_var_table);

    //We still want to return a ScopeExpr even for a single expression
    auto scope = Expr().addScope(range.getRange());
    // We add the expression to the scope
    auto& scope_ref = Expr(scope);
    auto& statements = scope_ref.as<ScopeExpr>()->getExprs();
    if (current() == TKN_COLON && accepts_single)
    {
      consume_current(); // :
      // TODO: check me
      auto range2 = startRange();

      statements.push_back(parse_statement());
      // Update the token range
      scope_ref.asBase()->setTokenRange(range2.getRange());
      return scope;
    }
    if (current() == TKN_LEFT_CURLY)
    {
      //Save '{' information
      auto lexeme_info = current();
      consume_current(); // '{'

      while (current() != TKN_RIGHT_CURLY && current() != TKN_EOF)
      {
        auto stt = parse_statement();
        statements.push_back(stt);

        /*if ((is_a<BreakContinueExpr>(stt) || is_a<FnReturnExpr>(stt))
          && current_tkn != TKN_RIGHT_CURLY)
          handle_unreachable_code();*/
      }
      if (check_consume(TKN_RIGHT_CURLY, "Unclosed curly bracket delimiter!").is_error())
        report<report_as::MESSAGE>(lexeme_info, nullptr, "Curly bracket opened here.");

      //If empty scope, push a no-op
      if (statements.is_empty())
        statements.push_back(Expr(Expr().addNOP(range.getRange())).asBase());

      // Update the token range
      scope_ref.asBase()->setTokenRange(range.getRange());
      return scope;
    }
    else
    {
      //TODO: choose consuming strategy
      report<report_as::ERROR>(current(), nullptr,
        "Expected the beginning of a scope ('{{'{}", accepts_single ? "or ':')!" : ")!");
      return Expr().addErrorStmt(range.getRange());
    }
  }

  StmtExprToken ASTMaker::parse_var_decl(bool is_global)
  {
    using enum Lexeme;
    auto depth = addDepth();
    auto range = startRange();
    auto panic = scopedSetPanic(&ASTMaker::panic_consume_semicolon);
    
    bool is_mut = false;
    if (is_global ?
      parse_global_var_mutability(is_mut).is_error() :
      parse_local_var_mutability(is_mut).is_error())
    {
      return Expr().addErrorStmt(range.getRange());
    }
    
    auto identifier = current();
    PROPAGATE_ERROR(
      check_consume(TKN_IDENTIFIER, current_panic, "Expected an identifier!").is_error(),
      Expr().addErrorStmt(getTokenBuffer().getRangeFrom(identifier))
    );

    StringView name = getTokenBuffer().getIdentifier(identifier);
    OptTok<TypeToken> var_type = None;
    if (current() == TKN_COLON)
    {
      auto type = parse_typename();
      PROPAGATE_ERROR(type, Expr().addErrorStmt(range.getRange()));
    }
    
    if (auto equal = current();
      check_consume(TKN_EQUAL, current_panic, "Expected a '='!").is_error())
      return Expr().addErrorStmt(getTokenBuffer().getRangeFrom(equal));
    
    OptTok<ProdExprToken> init = None;
    // TODO: handle default constructor?
    if (current() != TKN_KEYWORD_undefined)
    {
      auto rhs = parse_binary();
      auto& rhs_ref = Expr(rhs);
      PROPAGATE_ERROR(rhs_ref, Expr().addErrorStmt(rhs_ref.getTokenRange()));
      init = rhs;
      // If no type was specified, then the variable type is
      // the type of right hand side.
      if (var_type.isNone())
        var_type = rhs_ref.getType();
    }
    else // undefined
    {
      consume_current();
      if (is_global)
      {
        report<report_as::ERROR>(identifier, current_panic,
          "Global variables must be initialized!");
        return Expr().addErrorStmt(range.getRange());
      }
      if (var_type.isNone())
      {
        report<report_as::ERROR>(identifier, current_panic,
          "An uninitialized variable must have a type!");
        return Expr().addErrorStmt(range.getRange());
      }      
    }

    if (auto semicolon = current();
      check_consume(TKN_SEMICOLON, current_panic, "Expected a ';'!").is_error())
      return Expr().addErrorStmt(getTokenBuffer().getRangeFrom(semicolon));

    if (is_global)
    {
      return Expr().addGlobalDecl(
        range.getRange(), var_type.getValue(),
        name, init.getValue(), is_mut);
    }
    else
    {
      assert_true("Scope not set!", current_scope != nullptr);
      // Create the variable declaration
      // Its ID is its index into the current scope.
      auto decl = Expr().addVarDecl(range.getRange(), var_type.getValue(),
        local_var_table.size(), name, init, is_mut);
      auto decl_ptr = Expr(decl).as<VarDeclExpr>();
      // Save the variable initialization info
      local_var_table.push_back(LocalVarInfo{ name, *decl_ptr,
        init.isValue() ? VarStateFlag::INIT : VarStateFlag::UNDEF
        });
      // Register the current variable
      current_scope->getDecls().push_back(decl_ptr);
      return decl;
    }
  }

  OptTok<StmtExprToken> ASTMaker::parse_condition(bool is_elif)
  {
    using enum Lexeme;
    assert_true("Function should only be called if current_tkn == TKN_KEYWORD_(if/elif)",
      current() == TKN_KEYWORD_if || (is_elif && current() == TKN_KEYWORD_elif));
    auto depth = addDepth();
    auto range = startRange();

    consume_current(); //consume if (or elif if is_elif)

    auto if_cond = parse_binary_condition(); //if condition
    auto if_body = parse_scope(); //if body

    if (current() == TKN_KEYWORD_elif)
    {
      auto else_body = parse_condition(true); // we recurse
      return makeCondition(range.getRange(), if_cond, if_body, else_body);
    }
    OptTok<StmtExprToken> else_body = None;
    if (current() == TKN_KEYWORD_else)
    {
      consume_current(); //consume else
      else_body = parse_scope(); // else body
    }
    return makeCondition(range.getRange(), if_cond, if_body, else_body);
  }

  ExprBase* ASTMaker::parse_statement()
  {
    using enum Lexeme;
    //assert_true("Parse statement can only happen inside a function!", is_parsing_fn);
    auto depth = addDepth();
    auto range = startRange();

    bool is_valid = true; //modified by continue/break handling
    ExprBase* to_ret;
    switch (current())
    {
    case TKN_KEYWORD_var:
    {
      auto var = parse_var_decl();
      return Expr(var).asBase();
    }
    case TKN_LEFT_CURLY:
      return Expr(parse_scope(false)).asBase();
    case TKN_KEYWORD_if:
      if (auto cond = parse_condition(); cond.isValue())
        return Expr(cond.getValue()).asBase();
      return Expr(Expr().addNOP(range.getRange())).asBase();
      /*case TKN_KEYWORD_while:
        return parse_while();*/
      /*case TKN_KEYWORD_using:
        return parse_using(&ASTMaker::panic_consume_sttmnt);*/
      /*case TKN_KEYWORD_RETURN:
        return parse_return();*/

    case TKN_SEMICOLON:
      report<report_as::ERROR>(range.getRange(), nullptr, "Expected a statement!");
      consume_current(); // ';'
      return Expr(Expr().addError(range.getRange())).asBase();

      /*case TKN_KEYWORD_continue:
      case TKN_KEYWORD_break:
        return parse_continue_break(line_state);*/

        // EXPECTS ';'
    break; default:
    {
      auto panic = scopedSetPanic(&ASTMaker::panic_consume_semicolon);
      if (to_parse.getTokenBuffer().makeSourceInfo(current()).expr == "pass")
      {
        consume_current();
        to_ret = Expr(Expr().addNOP(range.getRange())).asBase();
      }
      else
        to_ret = Expr(parse_binary()).asBase();
    }
    }
    //TODO: recheck strategy
    if (check_consume(TKN_SEMICOLON, "Expected a ';'!").is_success())
      return to_ret;
    return Expr(Expr().addError(range.getRange())).asBase();
  }

  TypeToken ASTMaker::parse_typename() noexcept
  {
    using enum Lexeme;

    //Save current expression state
    auto depth = addDepth();
    auto range = startRange();

    // typeof(10 + 5) -> type of (10 + 5)
    if (current() == TKN_KEYWORD_typeof)
    {
      consume_current();
      return Expr(parse_parenthesis(&ASTMaker::parse_binary)).getType();
    }

    if (current() == TKN_KEYWORD_void)
    {
      consume_current(); //void
      return Type().getVoidType();
    }
    if (isBuiltinToken(current()))
    {
      auto type = current();
      consume_current();
      return Type().addBuiltin(KeywordToBuiltinID(type));
    }
    if (current() == TKN_KEYWORD_opaque)
    {
      consume_current();
      return Type().addOpaquePtr();
    }
    if (current() == TKN_KEYWORD_mutopaque)
    {
      consume_current();
      return Type().addMutOpaquePtr();
    }
    if (current() == TKN_KEYWORD_ptr || current() == TKN_KEYWORD_mutptr)
    {
      bool is_mut = current() == TKN_KEYWORD_mutptr;
      consume_current();

      if (check_consume(TKN_DOT, current_panic, "Expected a '.'!").is_success())
      {
        auto ptr_to = parse_typename();
        PROPAGATE_ERROR(ptr_to, ptr_to);
        return is_mut ? Type().addMutPtr(ptr_to) : Type().addPtr(ptr_to);
      }
      return Type().getErrorType();
    }
    report<report_as::ERROR>(range.getRange(), current_panic,
      "Expected a typename!");
    return Type().getErrorType();
  }

  ErrorFlag ASTMaker::parse_local_var_mutability(bool& is_mut) noexcept
  {
    using enum Lexeme;
    
    is_mut = false;
    if (current() == TKN_KEYWORD_var)
    {
      consume_current();
      is_mut = true;
      if (current() == TKN_KEYWORD_mut)
      {
        report_current<report_as::WARNING>(nullptr, "Unecessary 'mut' as 'var' is a shorthand for 'let mut'!");
        consume_current();
      }
      return ErrorFlag::success();
    }
    else if (current() == TKN_KEYWORD_let)
    {
      consume_current();
      if (current() == TKN_KEYWORD_mut)
      {
        is_mut = true;
        consume_current();
      }
      return ErrorFlag::success();
    }
    report_current<report_as::ERROR>(current_panic, "Expected a local variable declaration!");
    getReporter().message("A local variable declaration begins with 'var' or 'let'.");
    return ErrorFlag::error();
  }

  ErrorFlag ASTMaker::parse_global_var_mutability(bool& is_mut) noexcept
  {
    using enum Lexeme;

    is_mut = false;
    if (current() == TKN_KEYWORD_global)
    {
      consume_current();
      if (current() == TKN_KEYWORD_mut)
      {
        is_mut = true;
        consume_current();
      }
      return ErrorFlag::success();
    }
    report_current<report_as::ERROR>(current_panic, "Expected a global variable declaration!");
    getReporter().message("A global variable declaration begins with 'global' or 'global mut'.");
    return ErrorFlag::error();
  }

  OptTok<StmtExprToken> ASTMaker::decl_from_read(ProdExprToken expr) const noexcept
  {
    auto& read = Expr(expr);
    if (!read.isRead())
      return None;
    return read.as<ReadExpr>()->getDecl();
  }

  ProdExprToken ASTMaker::makeBinary(TokenRange range, ProdExprToken lhs, BinaryOp op, ProdExprToken rhs) noexcept
  {
    using enum BinarySupport;

    if (Expr(lhs).isError() || Expr(rhs).isError())
      return Expr().addError(range);

    auto& type = Type(lhs);
    auto support = type.supports(op, Type(rhs));
    switch_no_default(support)
    {
    case BUILTIN:
      // If the right hand side of the expression is a literal,
      // then we can check for division by zero.
      // If the left hand side is also a literal, we can constant fold
      // the expression
      if (auto rhs_p = Expr(rhs).as<LiteralExpr>();
        rhs_p != nullptr)
      {
        if (auto lhs_p = Expr(lhs).as<LiteralExpr>(); lhs_p != nullptr)
          return constantFold(range, *lhs_p, op, *rhs_p);
        if ((op == BinaryOp::OP_DIV || op == BinaryOp::OP_MOD) && isLiteralZero(rhs))
        {
          report<report_as::ERROR>(range, nullptr, "Integral division by zero is not allowed!");
          return Expr().addError(range);
        }
      }
      return Expr().addBinary(range, lhs, op, rhs);

    case INVALID_OP:
      report<report_as::ERROR>(range, nullptr,
        "'{}' does not support operator '{}'!",
        getTypeName(type), toStr(op));
      return Expr().addError(range);

    case INVALID_TYPE:
      report<report_as::ERROR>(range, nullptr,
        "'{}' does not support '{}' as right hand side of operator '{}'!",
        getTypeName(type), getTypeName(Type(rhs)), toStr(op));
      return Expr().addError(range);
    }
  }

  ProdExprToken ASTMaker::makeUnary(TokenRange range, UnaryOp op, ProdExprToken child) noexcept
  {
    using enum UnarySupport;

    if (Expr(child).isError())
      return Expr().addError(range);

    auto& type = Type(child);
    auto support = type.supports(op);

    switch_no_default(support)
    {
    case BUILTIN:
      if (auto ptr = Expr(child).as<LiteralExpr>(); ptr != nullptr)
        return constantFold(range, op, *ptr);
      return Expr().addUnary(range, op, child);
    case INVALID:
      report<report_as::ERROR>(range, current_panic,
        "'{}' does not support unary operator '{}'!",
        getTypeName(type), toStr(op));
      return Expr().addError(range);
    }
  }

  bool ASTMaker::isInvalidChain(ComparisonSet old, ComparisonSet new_set) const noexcept
  {
    using enum ComparisonSet;
    return old != new_set ||
      (old == NONE && new_set == NONE);
  }

  ProdExprToken ASTMaker::makeCast(TokenRange range, ProdExprToken to_cast, TypeToken to, bool is_bit_cast) noexcept
  {
    using enum ConversionSupport;
    PROPAGATE_ERROR(to_cast, to_cast);
    if (is_bit_cast)
    {
      unreachable("Not implemented!");
      return Expr().addError(range);
    }

    auto support = Type(to_cast).castableTo(Type(to));
    switch_no_default(support)
    {
    case BUILTIN:
    {
      auto builtin_t = Type(to).as<BuiltinType>();
      if (auto ptr = Expr(to_cast).as<LiteralExpr>(); ptr != nullptr && builtin_t != nullptr)
        return constantFold(range, *ptr, *builtin_t);
      return Expr().addCast(range, to, to_cast);
    }
    case INVALID:
      report<report_as::ERROR>(range, nullptr, "'{}' cannot be casted to '{}'!",
        getTypeName(Type(to_cast)), getTypeName(to));
      return Expr().addError(range);
    }
  }

  run::TypeOp BuiltinToTypeOp(BuiltinID ID) noexcept
  {
    using enum clt::run::TypeOp;

    static constexpr std::array ID_to_type =
    {
      u8_t, u8_t,
      u8_t, u16_t, u32_t, u64_t,
      i8_t, i16_t, i32_t, i64_t,
      f32_t, f64_t,
      u8_t, u16_t, u32_t, u64_t,
    };
    return ID_to_type[(u8)ID];
  }

  ProdExprToken ASTMaker::constantFold(TokenRange range, const LiteralExpr& lhs, BinaryOp op, const LiteralExpr& rhs) noexcept
  {

    static constexpr std::array table =
    {
      &run::NT_add, &run::NT_sub, &run::NT_mul, &run::NT_div, &run::NT_mod,
      &run::NT_bit_and, &run::NT_bit_or, &run::NT_bit_xor, &run::NT_shl, &run::NT_shr,
      &run::NT_bit_and, &run::NT_bit_or,
      &run::NT_le, &run::NT_leq, &run::NT_ge, &run::NT_geq, &run::NT_neq, &run::NT_eq
    };

    assert_true("Expected built-in type!", Type(lhs).is<BuiltinType>(), Type(rhs).is<BuiltinType>());
    const auto typeID = Type(lhs).as<BuiltinType>()->typeID();

    auto fn = table[static_cast<u8>(op)];
    auto [res, err] = fn(lhs.getValue(), rhs.getValue(),
      BuiltinToTypeOp(typeID));

    if (err == run::DIV_BY_ZERO)
    {
      report<report_as::ERROR>(range, nullptr,
        "Integral division by zero is not allowed!");
      return Expr().addError(range);
    }
    else if (warnFor(err))
    {
      report<report_as::WARNING>(range, nullptr,
        "{}", run::toExplanation(err));
    }

    const auto family = FamilyOf(op);
    // If this is true then the resulting expression is a boolean
    const bool is_ret_bool = family == OpFamily::BOOL_LOGIC || family == OpFamily::COMPARISON;

    return Expr().addLiteral(range, res,
      is_ret_bool ? BuiltinID::BOOL : typeID
    );
  }

  OptTok<StmtExprToken> ASTMaker::makeCondition(TokenRange range, ProdExprToken condition, StmtExprToken if_stmt, OptTok<StmtExprToken> else_stmt) noexcept
  {
    assert_true("Expected boolean type!", Type(condition).isBuiltinAnd(&isBool));
    if (auto literal = Expr(condition).as<LiteralExpr>(); literal != nullptr)
      return literal->getValue().is_none_set() ? else_stmt : OptTok<StmtExprToken>{ if_stmt };
    return Expr().addCondition(range, condition, if_stmt, else_stmt);
  }

  ProdExprToken ASTMaker::constantFold(TokenRange range, UnaryOp op, const LiteralExpr& lhs) noexcept
  {
    using enum clt::lng::UnaryOp;

    switch_no_default(op)
    {
    case OP_NEGATE:
    {
      const auto ID = Type(lhs).as<BuiltinType>()->typeID();
      auto [result, err] = run::NT_neg(lhs.getValue(), BuiltinToTypeOp(ID));
      if (warnFor(err))
        report<report_as::WARNING>(range, nullptr, "{}", run::toExplanation(err));
      return Expr().addLiteral(range, result, ID);
    }
    case OP_BOOL_NOT:
    {
      // No errors are possible
      auto [result, err] = run::bool_not(lhs.getValue());
      return Expr().addLiteral(range, result, BuiltinID::BOOL);
    }
    case OP_BIT_NOT:
    {
      // No errors are possible
      const auto ID = Type(lhs).as<BuiltinType>()->typeID();
      auto [result, err] = run::NT_bit_not(lhs.getValue(), BuiltinToTypeOp(ID));
      return Expr().addLiteral(range, result, ID);
    }
    }
  }

  ProdExprToken ASTMaker::constantFold(TokenRange range, const LiteralExpr& to_conv, const BuiltinType& to) noexcept
  {
    auto [result, err] = run::NT_cnv(to_conv.getValue(),
      BuiltinToTypeOp(Type(to_conv).as<BuiltinType>()->typeID()), BuiltinToTypeOp(to.typeID()));
    if (warnFor(err))
      report<report_as::WARNING>(range, nullptr, "{}", run::toExplanation(err));
    return Expr().addLiteral(range, result, to.typeID());
  }

  bool ASTMaker::isLiteralZero(ProdExprToken expr) const noexcept
  {
    return Type(expr).isBuiltinAnd(&isIntegral)
      && Expr(expr).as<LiteralExpr>()->getValue().is_none_set();
  }
  
  void PrintExpr(const ExprBase* tkn, const ParsedUnit& unit, u64 depth) noexcept
  {
    using enum ExprID;
    auto& buffer = unit.getExprBuffer();
    auto& types = unit.getProgram().getTypes();
    auto& tkn_buffer = unit.getTokenBuffer();
    auto& expr = *tkn;
    
    auto info = tkn_buffer.makeSourceInfo(expr.getTokenRange());
    switch (expr.classof())
    {
    break; case EXPR_ERROR:
      io::print("{}{:^{}}({:h}: {}){}", io::BrightRedF, "", depth * 3, expr.classof(), info.expr, io::Reset);

    break; case EXPR_LITERAL:
      io::print("{}{:^{}}({:h}: {}, {} {}){}", io::BrightGreenF, "", depth * 3, expr.classof(),
        info.expr, fmt_TypedQWORD{ static_cast<const LiteralExpr*>(&expr)->getValue(), types.getType(expr.getType()).as<BuiltinType>()->typeID() },
        types.getTypeName(expr.getType()), io::Reset);

    break; case EXPR_UNARY:
    {
      auto ptr = static_cast<const UnaryExpr*>(&expr);
      
      io::print("{}{:^{}}({:h}: '{:h}'", io::YellowF, "", depth * 3, expr.classof(),
        ptr->getOp());
      PrintExpr(ptr->getExpr(), unit, depth + 1);
      io::print("{}{:^{}}{}){}", io::YellowF, "", depth * 3, types.getTypeName(expr.getType()), io::Reset);
    }

    break; case EXPR_BINARY:
    {
      auto ptr = static_cast<const BinaryExpr*>(&expr);
      
      io::print("{}{:^{}}({:h}:", io::BrightCyanF, "", depth * 3, expr.classof());
      PrintExpr(ptr->getLHS(), unit, depth + 1);
      io::print("{}{:^{}} {:h}", io::BrightCyanF, "", depth * 3,
        ptr->getOp());
      PrintExpr(ptr->getRHS(), unit, depth + 1);
      io::print("{}{:^{}}{}){}", io::BrightCyanF, "", depth * 3, types.getTypeName(expr.getType()), io::Reset);
    }

    break; case EXPR_CAST:
    {
      auto ptr = static_cast<const CastExpr*>(&expr);

      io::print("{}{:^{}}({:h}: '{}' -> '{}'", io::BrightMagentaF, "", depth * 3, expr.classof(),
        types.getTypeName(ptr->getType()), types.getTypeName(ptr->getTypeToCastTo()));
      PrintExpr(ptr->getToCast(), unit, depth + 1);
      io::print("{}{:^{}}{}){}", io::BrightMagentaF, "", depth * 3, types.getTypeName(ptr->getType()), io::Reset);
    }

    break; default:
      io::print("{:^{}}{:h}", "", depth * 3, expr.classof());
      break;
    }
  }

  void PrintExpr(ProdExprToken tkn, const ParsedUnit& unit, u64 depth) noexcept
  {
    using enum ExprID;
    auto& buffer = unit.getExprBuffer();
    auto& expr = buffer.getExpr(tkn);
    PrintExpr(expr.asBase(), unit, depth);
  }
}

#undef SCOPED_SAVE_VECTOR