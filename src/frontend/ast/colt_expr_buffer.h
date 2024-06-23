/*****************************************************************/ /**
 * @file   colt_expr_buffer.h
 * @brief  Contains ExprBuffer, the class responsible of the
 * lifetimes of all expressions.
 * As with types, a ProdExprToken/StmtExprToken is returned
 * rather than a pointer or a reference.
 *
 * @author RPC
 * @date   April 2024
 *********************************************************************/
#ifndef HG_COLT_EXPR_BUFFER
#define HG_COLT_EXPR_BUFFER

#include "ast/colt_expr.h"

namespace clt::lng
{
  /// @brief Represents any expression that produces a value (or writes).
  class ProdExprVariant
  {
    MAKE_UNION_AND_GET_MEMBER(COLTC_PROD_EXPR_LIST);

    template<typename... Args>
    constexpr bool is_classof_any_of(meta::type_list<Args...>) const noexcept
    {
      return (... || (classof() == TypeToExprID<Args>()));
    }

  public:
    template<ProducerExpr Type, typename... Args>
    /// @brief Constructor
    /// @param args... The arguments to forward to the constructor
    constexpr ProdExprVariant(std::type_identity<Type>, Args&&... args) noexcept(
        std::is_nothrow_constructible_v<Type, Args...>)
    {
      construct<Type>(&getUnionMember<Type>(), std::forward<Args>(args)...);
    }

    /// @brief Returns the type of the current expression
    /// @return The type of the current expression
    constexpr TypeToken type() const noexcept
    {
      return std::bit_cast<ExprBase>(_ErrorExpr).type();
    }

    /// @brief Returns the range of tokens representing the current expression
    /// @return The range of tokens
    constexpr TokenRange token_range() const noexcept
    {
      return std::bit_cast<ExprBase>(_ErrorExpr).token_range();
    }

    /// @brief Returns the expression ID
    /// @return The expression ID (used for downcasts)
    constexpr ExprID classof() const noexcept
    {
      return std::bit_cast<ExprBase>(_ErrorExpr).classof();
    }

    template<ProducerExpr T>
    /// @brief Downcasts the variant to 'T'
    /// @return nullptr if type does not match else pointer to the type
    constexpr T* as() noexcept
    {
      if (classof() != TypeToExprID<T>())
        return nullptr;
      return &getUnionMember<T>();
    }

    template<ProducerExpr T>
    /// @brief Downcasts the variant to 'T'
    /// @return nullptr if type does not match else pointer to the type
    constexpr const T* as() const noexcept
    {
      if (classof() != TypeToExprID<T>())
        return nullptr;
      return &getUnionMember<T>();
    }

    template<typename T>
    /// @brief Downcasts the variant to 'T'
    /// @return nullptr if type does not match else pointer to the type
    constexpr const T* as() const noexcept
    {
      static_assert(
          producer_group_requirements_t<T>::size != 0,
          "Group must be inherited from!");
      if (!is_classof_any_of(producer_group_requirements_t<T>{}))
        return nullptr;
      return ptr_to<const T*>(&_buffer);
    }

    template<typename T>
    /// @brief Downcasts the variant to 'T'
    /// @return nullptr if type does not match else pointer to the type
    constexpr T* as() noexcept
    {
      static_assert(
          producer_group_requirements_t<T>::size != 0,
          "Group must be inherited from!");
      if (!is_classof_any_of(producer_group_requirements_t<T>{}))
        return nullptr;
      return ptr_to<T*>(&_buffer);
    }

    /// @brief Returns the expression as an ExprBase pointer
    /// @return ExprBase* (never null)
    constexpr ExprBase* as_base() noexcept { return ptr_to<ExprBase*>(&_buffer); }

    /// @brief Returns the expression as an ExprBase pointer
    /// @return ExprBase* (never null)
    constexpr const ExprBase* as_base() const noexcept
    {
      return ptr_to<const ExprBase*>(&_buffer);
    }

    template<ProducerExpr Ty>
    /// @brief Check if the stored expression is a 'Ty'
    /// @return True if the stored expression is a 'Ty'
    constexpr bool is() const noexcept
    {
      return classof() == TypeToExprID<Ty>();
    }

    /// @brief Check if the current expression is an error
    /// @return True if error
    constexpr bool is_error() const noexcept
    {
      return classof() == ExprID::EXPR_ERROR;
    }
    /// @brief Check if the current expression is a read var/global
    /// @return True if VarReadExpr or GlobalReadExpr
    constexpr bool is_read() const noexcept
    {
      return classof() == ExprID::EXPR_VAR_READ
             || classof() == ExprID::EXPR_GLOBAL_READ;
    }
  };

  /// @brief Variant of statement
  class StmtExprVariant
  {
    MAKE_UNION_AND_GET_MEMBER(COLTC_STMT_EXPR_LIST);

  public:
    template<StatementExpr Type, typename... Args>
    /// @brief Constructor
    /// @param args... The arguments to forward to the constructor
    constexpr StmtExprVariant(std::type_identity<Type>, Args&&... args) noexcept(
        std::is_nothrow_constructible_v<Type, Args...>)
    {
      construct<Type>(&getUnionMember<Type>(), std::forward<Args>(args)...);
    }

    /// @brief Returns the type of the current expression
    /// @return The type of the current expression
    constexpr TypeToken type() const noexcept
    {
      // UB...
      return (ptr_to<const ExprBase*>(&_buffer))->type();
    }

    /// @brief Returns the range of tokens representing the current expression
    /// @return The range of tokens
    constexpr TokenRange token_range() const noexcept
    {
      // UB...
      return (ptr_to<const ExprBase*>(&_buffer))->token_range();
    }

    /// @brief Returns the expression ID
    /// @return The expression ID (used for downcasts)
    constexpr ExprID classof() const noexcept
    {
      // UB...
      return (ptr_to<const ExprBase*>(&_buffer))->classof();
    }

    template<StatementExpr T>
    /// @brief Downcasts the variant to 'T'
    /// @return nullptr if type does not match else pointer to the type
    constexpr T* as() noexcept
    {
      if (classof() != TypeToExprID<T>())
        return nullptr;
      return &getUnionMember<T>();
    }

    template<StatementExpr T>
    /// @brief Downcasts the variant to 'T'
    /// @return nullptr if type does not match else pointer to the type
    constexpr const T* as() const noexcept
    {
      if (classof() != TypeToExprID<T>())
        return nullptr;
      return &getUnionMember<T>();
    }

    template<StatementExpr Ty>
    /// @brief Check if the stored expression is a 'Ty'
    /// @return True if the stored expression is a 'Ty'
    constexpr bool is() const noexcept
    {
      return classof() == TypeToExprID<Ty>();
    }

    /// @brief Check if the current expression is a local variable declaration
    /// @return True if classof() returns EXPR_VAR_DECL
    constexpr bool is_var_decl() const noexcept
    {
      return classof() == ExprID::EXPR_VAR_DECL;
    }
    /// @brief Check if the current expression is a local variable declaration
    /// @return True if classof() returns EXPR_GLOBAL_DECL
    constexpr bool is_global_decl() const noexcept
    {
      return classof() == ExprID::EXPR_GLOBAL_DECL;
    }
    /// @brief Check if the current expression is a scope
    /// @return True if classof() returns EXPR_SCOPE
    constexpr bool is_scope() const noexcept
    {
      return classof() == ExprID::EXPR_SCOPE;
    }

    /// @brief Returns the expression as an ExprBase pointer
    /// @return ExprBase* (never null)
    constexpr ExprBase* as_base() noexcept { return ptr_to<ExprBase*>(&_buffer); }

    /// @brief Returns the expression as an ExprBase pointer
    /// @return ExprBase* (never null)
    constexpr const ExprBase* as_base() const noexcept
    {
      return ptr_to<const ExprBase*>(&_buffer);
    }

    /// @brief Destructor
    ~StmtExprVariant()
    {
      if (classof() == ExprID::EXPR_SCOPE)
        getUnionMember<ScopeExpr>().~ScopeExpr();
    }
  };

  // Ensure all the types are divided between Prod and Stmt (-1 as ErrorExpr is in both)
  static_assert(
      meta::type_list<COLTC_PROD_EXPR_LIST>::size
              + meta::type_list<COLTC_STMT_EXPR_LIST>::size - 1
          == meta::type_list<COLTC_EXPR_LIST>::size,
      "Some types are missing from COLTC_PROD_EXPR_LIST or COLTC_STMT_EXPR_LIST");

  /// @brief Class responsible of the lifetimes of all expressions
  class ExprBuffer
  {
    /// @brief The buffer of types
    TypeBuffer& types;
    /// @brief The list of ProducerExpr
    FlatList<ProdExprVariant, 512> prod_expr{};
    /// @brief The list of StatementExpr
    FlatList<StmtExprVariant, 512> stmt_expr{};

    /// @brief Returns the next ProdExprToken.
    /// A push_back to prod_expr must follow this call.
    /// @return The next ProdExprToken
    ProdExprToken next_prod() noexcept
    {
      assert_true("Integer overflow!", prod_expr.size() <= ProdExprToken::MAX_VALUE);
      return ProdExprToken{static_cast<u32>(prod_expr.size())};
    }

    template<ProducerExpr T, typename... Args>
    /// @brief Emplaces a new ProducerExpr at the end of 'prod_expr'
    /// @param args... The arguments used to forward to the constructor
    /// @return The ProdExprToken representing the new expression
    ProdExprToken add_new_prod(Args&&... args) noexcept
    {
      auto to_ret = next_prod();
      prod_expr.push_back(
          InPlace, std::type_identity<T>{}, std::forward<Args>(args)...);
      return to_ret;
    }

    /// @brief Returns the next StmtExprToken.
    /// A push_back to prod_expr must follow this call.
    /// @return The next ProdExprToken
    StmtExprToken next_stmt() noexcept
    {
      assert_true("Integer overflow!", stmt_expr.size() <= StmtExprToken::MAX_VALUE);
      return StmtExprToken{static_cast<u32>(stmt_expr.size())};
    }

    template<StatementExpr T, typename... Args>
    /// @brief Emplaces a new ProducerExpr at the end of 'prod_expr'
    /// @param args... The arguments used to forward to the constructor
    /// @return The ProdExprToken representing the new expression
    StmtExprToken add_new_stmt(Args&&... args) noexcept
    {
      auto to_ret = next_stmt();
      stmt_expr.push_back(
          InPlace, std::type_identity<T>{}, std::forward<Args>(args)...);
      return to_ret;
    }

  public:
    /// @brief Constructor
    /// @param types Reference to the global type buffer
    ExprBuffer(TypeBuffer& types) noexcept
        : types(types)
    {
    }

    /// @brief Returns the expression represented by 'prod'
    /// @param prod The producer expression token
    /// @return Reference to the expression represented by 'prod'
    ProdExprVariant& expr(ProdExprToken prod) noexcept
    {
      return prod_expr[prod.index];
    }
    /// @brief Returns the expression represented by 'prod'
    /// @param prod The producer expression token
    /// @return Reference to the expression represented by 'prod'
    const ProdExprVariant& expr(ProdExprToken prod) const noexcept
    {
      return prod_expr[prod.index];
    }

    /// @brief Returns the expression represented by 'stmt'
    /// @param stmt The statement expression token
    /// @return Reference to the expression represented by 'stmt'
    const StmtExprVariant& expr(StmtExprToken stmt) const noexcept
    {
      return stmt_expr[stmt.index];
    }
    /// @brief Returns the expression represented by 'stmt'
    /// @param stmt The statement expression token
    /// @return Reference to the expression represented by 'stmt'
    StmtExprVariant& expr(StmtExprToken stmt) noexcept
    {
      return stmt_expr[stmt.index];
    }

    /// @brief Returns the type of an expression
    /// @param prod The producer expression token
    /// @return Type of the expression represented by 'prod'
    TypeToken type_token(ProdExprToken prod) const noexcept
    {
      return expr(prod).type();
    }

    /// @brief Returns the type of an expression
    /// @param prod The producer expression token
    /// @return Type of the expression represented by 'prod'
    const TypeVariant& type(ProdExprToken prod) const noexcept
    {
      return types.type(type_token(prod));
    }

    /// @brief Returns the type represented by 'type'
    /// @param type The type token
    /// @return Type represented by the token
    const TypeVariant& type(TypeToken type) const noexcept
    {
      return types.type(type);
    }

    /// @brief Creates an error expression
    /// @param range The range of tokens
    /// @return ErrorExpr
    ProdExprToken add_error(TokenRange range) noexcept
    {
      return add_new_prod<ErrorExpr>(range, types.error_type());
    }

    /// @brief Creates an error statement
    /// @param range The range of tokens
    /// @return ErrorExpr
    StmtExprToken add_error_stmt(TokenRange range) noexcept
    {
      return add_new_stmt<ErrorExpr>(range, types.error_type());
    }

    /// @brief Creates a no-op expression
    /// @param range The range of tokens
    /// @return NOPExpr
    ProdExprToken add_nop(TokenRange range) noexcept
    {
      return add_new_prod<NOPExpr>(range, types.void_type());
    }

    /// @brief Creates a literal expression
    /// @param range The range of tokens
    /// @param value The value of the literal
    /// @param type The literal type
    /// @return LiteralExpr
    ProdExprToken add_literal(
        TokenRange range, QWORD_t value, BuiltinID type) noexcept
    {
      return add_new_prod<LiteralExpr>(range, types.add_builtin(type), value);
    }

    /// @brief Creates a unary expression
    /// @param range The range of tokens
    /// @param op The unary operation
    /// @param expr The expression on which the unary operator is applied
    /// @return UnaryExpr
    ProdExprToken add_unary(
        TokenRange range, UnaryOp op, ProdExprToken expr) noexcept
    {
      return add_new_prod<UnaryExpr>(range, type_token(expr), op, expr);
    }

    /// @brief Creates a binary expression.
    /// The type of the resulting expression is correctly deduced to 'bool'
    /// if the 'op' is a comparison token.
    /// @param range The range of tokens
    /// @param lhs The left hand side of the expression
    /// @param op The binary operation
    /// @param rhs The right hand side of the expression
    /// @return BinaryExpr
    ProdExprToken add_binary(
        TokenRange range, ProdExprToken lhs, BinaryOp op, ProdExprToken rhs) noexcept
    {
      assert_true(
          "Both expression must be of the same type!",
          type_token(lhs) == type_token(rhs));
      return add_new_prod<BinaryExpr>(
          range,
          family_of(op) == OpFamily::COMPARISON ? types.add_builtin(BuiltinID::BOOL)
                                                : type_token(lhs),
          lhs, op, rhs);
    }

    /// @brief Creates a cast
    /// @param range The range of tokens
    /// @param cast_to The type to cast to
    /// @param to_cast The expression to cast
    /// @return CastExpr
    ProdExprToken add_cast(
        TokenRange range, TypeToken cast_to, ProdExprToken to_cast) noexcept
    {
      assert_true(
          "Both expression must be of built-in type!", type(cast_to).is_builtin(),
          type(cast_to).is_builtin());
      return add_new_prod<CastExpr>(range, cast_to, to_cast, false);
    }

    /// @brief Creates a bit cast.
    /// At least one of the types must be byte type.
    /// @param range The range of tokens
    /// @param cast_to The type to cast to
    /// @param to_cast The expression to cast
    /// @return CastExpr
    ProdExprToken add_bit_cast(
        TokenRange range, TypeToken cast_to, ProdExprToken to_cast) noexcept
    {
      assert_true(
          "Both expression must be of built-in type!", type(cast_to).is_builtin(),
          type(cast_to).is_builtin());
      assert_true(
          "Both expression must be of built-in type!",
          type(cast_to).is_builtin_and(&is_bytes)
              || type(cast_to).is_builtin_and(&is_bytes));
      return add_new_prod<CastExpr>(range, cast_to, to_cast, true);
    }

    /// @brief Creates an address of expression.
    /// @param range The range of tokens
    /// @param decl The variable declaration whose address to return
    /// @return AddressOfExpr
    ProdExprToken add_address_of(TokenRange range, StmtExprToken decl) noexcept
    {
      auto& ref = expr(decl);
      if (auto ptr = ref.as<VarDeclExpr>(); ptr)
        return add_new_prod<AddressOfExpr>(
            range,
            ptr->is_mut() ? types.add_mut_ptr(ptr->type())
                          : types.add_ptr(ptr->type()),
            decl);
      if (auto ptr = ref.as<GlobalDeclExpr>(); ptr)
        return add_new_prod<AddressOfExpr>(
            range,
            ptr->is_mut() ? types.add_mut_ptr(ptr->type())
                          : types.add_ptr(ptr->type()),
            decl);

      clt::unreachable("Expected a variable declaration!");
    }

    /// @brief Creates a pointer load.
    /// @param range The range of tokens
    /// @param to_cast The expression from which to load (of pointer type)
    /// @return PtrLoadExpr
    ProdExprToken add_ptr_load(TokenRange range, ProdExprToken to_cast) noexcept
    {
      assert_true(
          "Expression must have a pointer type!",
          type(to_cast).is_any_ptr() && !type(to_cast).is_any_opaque_ptr());
      auto& ref = type(to_cast);
      if (auto ptr = ref.as<MutPtrType>(); ptr)
        return add_new_prod<PtrLoadExpr>(range, ptr->pointing_to(), to_cast);
      if (auto ptr = ref.as<PtrType>(); ptr)
        return add_new_prod<PtrLoadExpr>(range, ptr->pointing_to(), to_cast);
      clt::unreachable("Invalid type!");
    }

    /// @brief Creates a read from a variable.
    /// @param range The range of tokens
    /// @param var_decl The variable declaration from which to read
    /// @return VarReadExpr
    ProdExprToken add_var_read(TokenRange range, StmtExprToken var_decl) noexcept
    {
      assert_true("Expected a VarDeclExpr!", expr(var_decl).is_var_decl());
      return add_new_prod<VarReadExpr>(range, expr(var_decl).type(), var_decl);
    }

    /// @brief Creates a read from a variable.
    /// @param range The range of tokens
    /// @param var_decl The variable declaration from which to read
    /// @return GlobalReadExpr
    ProdExprToken add_global_read(TokenRange range, StmtExprToken var_decl) noexcept
    {
      assert_true("Expected a GlobalDeclExpr!", expr(var_decl).is_global_decl());
      return add_new_prod<GlobalReadExpr>(range, expr(var_decl).type(), var_decl);
    }

    ProdExprToken add_fn_call() noexcept
    {
      // TODO: add body
      unreachable("NOT IMPLEMENTED!");
    }

    /// @brief Creates a write to a variable
    /// @param range The range of tokens
    /// @param var_decl The variable declaration
    /// @param value The value to write
    /// @return VarWriteExpr
    ProdExprToken add_var_write(
        TokenRange range, StmtExprToken var_decl, ProdExprToken value) noexcept
    {
      assert_true("Expected a VarDeclExpr!", expr(var_decl).is_var_decl());
      assert_true("Types must match!", expr(var_decl).type() == type_token(value));
      return add_new_prod<VarWriteExpr>(range, types.void_type(), var_decl, value);
    }

    /// @brief Creates a write to a global variable
    /// @param range The range of tokens
    /// @param var_decl The variable declaration
    /// @param value The value to write
    /// @return GlobalWriteExpr
    ProdExprToken add_global_write(
        TokenRange range, StmtExprToken var_decl, ProdExprToken value) noexcept
    {
      assert_true("Expected a GlobalDeclExpr!", expr(var_decl).is_global_decl());
      assert_true("Types must match!", expr(var_decl).type() == type_token(value));
      return add_new_prod<GlobalWriteExpr>(
          range, types.void_type(), var_decl, value);
    }

    /// @brief Creates a store to a pointer
    /// @param range The range of tokens
    /// @param write_to The pointer to which to write
    /// @param to_write The value to write to that pointer
    /// @return PtrStoreExpr
    ProdExprToken add_ptr_store(
        TokenRange range, ProdExprToken write_to, ProdExprToken to_write) noexcept
    {
      assert_true(
          "Expected a non-opaque pointer to mutable type!",
          type(write_to).is_mut_ptr() && !type(write_to).is_any_opaque_ptr());
      assert_true(
          "Types must match!",
          type(write_to).as<MutPtrType>()->pointing_to() == type_token(to_write));
      return add_new_prod<PtrStoreExpr>(
          range, types.void_type(), write_to, to_write);
    }

    /// @brief Creates a move from a variable to another
    /// @param range The range of tokens
    /// @param from The variable to move from
    /// @param to The variable to move to
    /// @return MoveExpr
    ProdExprToken add_move(
        TokenRange range, StmtExprToken from, StmtExprToken to) noexcept
    {
      assert_true(
          "Expected two local variable declarations!", expr(from).is_var_decl(),
          expr(to).is_var_decl());
      return add_new_prod<MoveExpr>(range, types.void_type(), from, to);
    }

    /// @brief Creates a copy from a variable to another
    /// @param range The range of tokens
    /// @param from The variable to copy from
    /// @param to The variable to copy to
    /// @return CopyExpr
    ProdExprToken add_copy(
        TokenRange range, StmtExprToken from, StmtExprToken to) noexcept
    {
      assert_true(
          "Expected two variable declarations!",
          expr(from).is_var_decl() || expr(from).is_global_decl(),
          expr(to).is_var_decl() || expr(to).is_global_decl());
      return add_new_prod<CopyExpr>(range, types.void_type(), from, to);
    }

    /// @brief Creates a conditional move from a variable to another
    /// @param range The range of tokens
    /// @param from The variable to conditionally move from
    /// @param to The variable to move to
    /// @return CMoveExpr
    ProdExprToken add_cmove(
        TokenRange range, StmtExprToken from, StmtExprToken to) noexcept
    {
      assert_true(
          "Expected two variable declarations!",
          expr(from).is_var_decl() || expr(from).is_global_decl(),
          expr(to).is_var_decl() || expr(to).is_global_decl());
      return add_new_prod<CMoveExpr>(range, types.void_type(), from, to);
    }

    /// @brief Creates a scope
    /// @param range The range of tokens
    /// @return ScopeExpr
    StmtExprToken add_scope(TokenRange range) noexcept
    {
      return add_new_stmt<ScopeExpr>(range, types.void_type());
    }

    /// @brief Creates a scope
    /// @param range The range of tokens
    /// @param parent The parent of the scope
    /// @return ScopeExpr
    StmtExprToken add_scope(TokenRange range, StmtExprToken parent) noexcept
    {
      assert_true("Expected a scope as a parent!", expr(parent).is_scope());
      return add_new_stmt<ScopeExpr>(range, types.void_type(), parent);
    }

    /// @brief Creates a condition
    /// @param range The range of tokens
    /// @param if_cond The if condition
    /// @param if_stmt The if statement
    /// @param else_stmt The else statement
    /// @return ConditionExpr
    StmtExprToken add_condition(
        TokenRange range, ProdExprToken if_cond, StmtExprToken if_stmt,
        OptTok<StmtExprToken> else_stmt) noexcept
    {
      assert_true("Expected bool type!", type(if_cond).is_builtin_and(&is_bool));
      return add_new_stmt<ConditionExpr>(
          range, types.void_type(), if_cond, if_stmt, else_stmt);
    }

    /// @brief Creates a global variable declaration
    /// @param range The range of tokens
    /// @param type The type of the variable
    /// @param name The name of the global
    /// @param init The initial value of the global
    /// @param is_mut True if mutable
    /// @return GlobalDeclExpr
    StmtExprToken add_global_decl(
        TokenRange range, TypeToken type, StringView name, ProdExprToken init,
        bool is_mut) noexcept
    {
      return add_new_stmt<GlobalDeclExpr>(range, type, name, init, is_mut);
    }

    /// @brief Creates a local variable declaration
    /// @param range The range of tokens
    /// @param type The type of the variable
    /// @param local_id The local ID
    /// @param name The name of the variable
    /// @param init The initial value of the global
    /// @param is_mut True if mutable
    /// @return VarDeclExpr
    StmtExprToken add_var_decl(
        TokenRange range, TypeToken type, u32 local_id, StringView name,
        OptTok<ProdExprToken> init, bool is_mut) noexcept
    {
      return add_new_stmt<VarDeclExpr>(range, type, local_id, name, init, is_mut);
    }
  };
} // namespace clt::lng

#endif // !HG_COLT_EXPR_BUFFER
