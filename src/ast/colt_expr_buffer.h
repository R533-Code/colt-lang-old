/*****************************************************************//**
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
    constexpr ProdExprVariant(std::type_identity<Type>, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<Type, Args...>)
    {
      construct<Type>(&getUnionMember<Type>(), std::forward<Args>(args)...);
    }

    /// @brief Returns the type of the current expression
    /// @return The type of the current expression
    constexpr TypeToken getType() const noexcept
    {
      return std::bit_cast<ExprBase>(_ErrorExpr).getType();
    }

    /// @brief Returns the range of tokens representing the current expression
    /// @return The range of tokens
    constexpr TokenRange getTokenRange() const noexcept
    {
      return std::bit_cast<ExprBase>(_ErrorExpr).getTokenRange();
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
      static_assert(producer_group_requirements_t<T>::size != 0, "Group must be inherited from!");
      if (!is_classof_any_of(producer_group_requirements_t<T>{}))
        return nullptr;
      return (const T*)&_buffer;
    }
    
    template<typename T>
    /// @brief Downcasts the variant to 'T'
    /// @return nullptr if type does not match else pointer to the type
    constexpr T* as() noexcept
    {
      static_assert(producer_group_requirements_t<T>::size != 0, "Group must be inherited from!");
      if (!is_classof_any_of(producer_group_requirements_t<T>{}))
        return nullptr;
      return (T*)&_buffer;
    }

    /// @brief Returns the expression as an ExprBase pointer
    /// @return ExprBase* (never null)
    constexpr ExprBase* asBase() noexcept
    {
      return (ExprBase*)&_buffer;
    }
    
    /// @brief Returns the expression as an ExprBase pointer
    /// @return ExprBase* (never null)
    constexpr const ExprBase* asBase() const noexcept
    {
      return (const ExprBase*)&_buffer;
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
    constexpr bool isError() const noexcept { return classof() == ExprID::EXPR_ERROR; }
    /// @brief Check if the current expression is a read var/global
    /// @return True if VarReadExpr or GlobalReadExpr
    constexpr bool isRead() const noexcept { return classof() == ExprID::EXPR_VAR_READ || classof() == ExprID::EXPR_GLOBAL_READ; }
  };

  /// @brief Variant of statement
  class StmtExprVariant
  {
    MAKE_UNION_AND_GET_MEMBER(COLTC_STMT_EXPR_LIST);

  public:
    template<StatementExpr Type, typename... Args>
    /// @brief Constructor
    /// @param args... The arguments to forward to the constructor
    constexpr StmtExprVariant(std::type_identity<Type>, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<Type, Args...>)
    {
      construct<Type>(&getUnionMember<Type>(), std::forward<Args>(args)...);
    }

    /// @brief Returns the type of the current expression
    /// @return The type of the current expression
    constexpr TypeToken getType() const noexcept
    {
      // UB...
      return ((const ExprBase*)&_buffer)->getType();
    }

    /// @brief Returns the range of tokens representing the current expression
    /// @return The range of tokens
    constexpr TokenRange getTokenRange() const noexcept
    {
      // UB...
      return ((const ExprBase*)&_buffer)->getTokenRange();
    }

    /// @brief Returns the expression ID
    /// @return The expression ID (used for downcasts)
    constexpr ExprID classof() const noexcept
    {
      // UB...
      return ((const ExprBase*)&_buffer)->classof();
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
    constexpr bool isVarDecl() const noexcept { return classof() == ExprID::EXPR_VAR_DECL; }
    /// @brief Check if the current expression is a local variable declaration
    /// @return True if classof() returns EXPR_GLOBAL_DECL
    constexpr bool isGlobalDecl() const noexcept { return classof() == ExprID::EXPR_GLOBAL_DECL; }
    /// @brief Check if the current expression is a scope
    /// @return True if classof() returns EXPR_SCOPE
    constexpr bool isScope() const noexcept { return classof() == ExprID::EXPR_SCOPE; }

    /// @brief Returns the expression as an ExprBase pointer
    /// @return ExprBase* (never null)
    constexpr ExprBase* asBase() noexcept
    {
      return (ExprBase*)&_buffer;
    }

    /// @brief Returns the expression as an ExprBase pointer
    /// @return ExprBase* (never null)
    constexpr const ExprBase* asBase() const noexcept
    {
      return (const ExprBase*)&_buffer;
    }

    /// @brief Destructor
    ~StmtExprVariant()
    {
      if (classof() == ExprID::EXPR_SCOPE)
        getUnionMember<ScopeExpr>().~ScopeExpr();
    }
  };

  // Ensure all the types are divided between Prod and Stmt (-1 as ErrorExpr is in both)
  static_assert(meta::type_list<COLTC_PROD_EXPR_LIST>::size + meta::type_list<COLTC_STMT_EXPR_LIST>::size - 1
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
    ProdExprToken getNextProd() noexcept
    {
      assert_true("Integer overflow!", prod_expr.size() <= ProdExprToken::MAX_VALUE);
      return ProdExprToken{ static_cast<u32>(prod_expr.size()) };
    }

    template<ProducerExpr T, typename... Args>
    /// @brief Emplaces a new ProducerExpr at the end of 'prod_expr'
    /// @param args... The arguments used to forward to the constructor
    /// @return The ProdExprToken representing the new expression
    ProdExprToken addNewProd(Args&&... args) noexcept
    {
      auto to_ret = getNextProd();
      prod_expr.push_back(InPlace, std::type_identity<T>{}, std::forward<Args>(args)...);
      return to_ret;
    }
    
    /// @brief Returns the next StmtExprToken.
    /// A push_back to prod_expr must follow this call.
    /// @return The next ProdExprToken
    StmtExprToken getNextStmt() noexcept
    {
      assert_true("Integer overflow!", stmt_expr.size() <= StmtExprToken::MAX_VALUE);
      return StmtExprToken{ static_cast<u32>(stmt_expr.size()) };
    }

    template<StatementExpr T, typename... Args>
    /// @brief Emplaces a new ProducerExpr at the end of 'prod_expr'
    /// @param args... The arguments used to forward to the constructor
    /// @return The ProdExprToken representing the new expression
    StmtExprToken addNewStmt(Args&&... args) noexcept
    {
      auto to_ret = getNextStmt();
      stmt_expr.push_back(InPlace, std::type_identity<T>{}, std::forward<Args>(args)...);
      return to_ret;
    }

  public:
    /// @brief Constructor
    /// @param types Reference to the global type buffer
    ExprBuffer(TypeBuffer& types) noexcept
      : types(types) {}

    /// @brief Returns the expression represented by 'prod'
    /// @param prod The producer expression token
    /// @return Reference to the expression represented by 'prod'
    ProdExprVariant& getExpr(ProdExprToken prod) noexcept { return prod_expr[prod.index]; }
    /// @brief Returns the expression represented by 'prod'
    /// @param prod The producer expression token
    /// @return Reference to the expression represented by 'prod'
    const ProdExprVariant& getExpr(ProdExprToken prod) const noexcept { return prod_expr[prod.index]; }

    /// @brief Returns the expression represented by 'stmt'
    /// @param stmt The statement expression token
    /// @return Reference to the expression represented by 'stmt'
    const StmtExprVariant& getExpr(StmtExprToken stmt) const noexcept { return stmt_expr[stmt.index]; }
    /// @brief Returns the expression represented by 'stmt'
    /// @param stmt The statement expression token
    /// @return Reference to the expression represented by 'stmt'
    StmtExprVariant& getExpr(StmtExprToken stmt) noexcept { return stmt_expr[stmt.index]; }

    /// @brief Returns the type of an expression
    /// @param prod The producer expression token
    /// @return Type of the expression represented by 'prod'
    TypeToken getTypeToken(ProdExprToken prod) const noexcept { return getExpr(prod).getType(); }

    /// @brief Returns the type of an expression
    /// @param prod The producer expression token
    /// @return Type of the expression represented by 'prod'
    const TypeVariant& getType(ProdExprToken prod) const noexcept { return types.getType(getTypeToken(prod)); }

    /// @brief Returns the type represented by 'type'
    /// @param type The type token
    /// @return Type represented by the token
    const TypeVariant& getType(TypeToken type) const noexcept { return types.getType(type); }

    /// @brief Creates an error expression
    /// @param range The range of tokens
    /// @return ErrorExpr
    ProdExprToken addError(TokenRange range) noexcept
    {
      return addNewProd<ErrorExpr>(range, types.getErrorType());
    }
    
    /// @brief Creates an error statement
    /// @param range The range of tokens
    /// @return ErrorExpr
    StmtExprToken addErrorStmt(TokenRange range) noexcept
    {
      return addNewStmt<ErrorExpr>(range, types.getErrorType());
    }
    
    /// @brief Creates a no-op expression
    /// @param range The range of tokens
    /// @return NOPExpr
    ProdExprToken addNOP(TokenRange range) noexcept
    {
      return addNewProd<NOPExpr>(range, types.getVoidType());
    }

    /// @brief Creates a literal expression
    /// @param range The range of tokens
    /// @param value The value of the literal
    /// @param type The literal type
    /// @return LiteralExpr
    ProdExprToken addLiteral(TokenRange range, QWORD_t value, BuiltinID type) noexcept
    {
      return addNewProd<LiteralExpr>(range, types.addBuiltin(type), value);
    }

    /// @brief Creates a unary expression
    /// @param range The range of tokens
    /// @param op The unary operation
    /// @param expr The expression on which the unary operator is applied
    /// @return UnaryExpr
    ProdExprToken addUnary(TokenRange range, UnaryOp op, ProdExprToken expr) noexcept
    {
      return addNewProd<UnaryExpr>(range, getTypeToken(expr), op, expr);
    }

    /// @brief Creates a binary expression.
    /// The type of the resulting expression is correctly deduced to 'bool'
    /// if the 'op' is a comparison token.
    /// @param range The range of tokens
    /// @param lhs The left hand side of the expression
    /// @param op The binary operation
    /// @param rhs The right hand side of the expression
    /// @return BinaryExpr
    ProdExprToken addBinary(TokenRange range, ProdExprToken lhs, BinaryOp op, ProdExprToken rhs) noexcept
    {
      assert_true("Both expression must be of the same type!", getTypeToken(lhs) == getTypeToken(rhs));
      return addNewProd<BinaryExpr>(range,
        FamilyOf(op) == OpFamily::COMPARISON ? types.addBuiltin(BuiltinID::BOOL) : getTypeToken(lhs),
        lhs, op, rhs);
    }

    /// @brief Creates a cast
    /// @param range The range of tokens
    /// @param cast_to The type to cast to
    /// @param to_cast The expression to cast
    /// @return CastExpr
    ProdExprToken addCast(TokenRange range, TypeToken cast_to, ProdExprToken to_cast) noexcept
    {
      assert_true("Both expression must be of built-in type!",
        getType(cast_to).isBuiltin(), getType(cast_to).isBuiltin());
      return addNewProd<CastExpr>(range, cast_to, to_cast, false);
    }

    /// @brief Creates a bit cast.
    /// At least one of the types must be byte type.
    /// @param range The range of tokens
    /// @param cast_to The type to cast to
    /// @param to_cast The expression to cast
    /// @return CastExpr
    ProdExprToken addBitCast(TokenRange range, TypeToken cast_to, ProdExprToken to_cast) noexcept
    {
      assert_true("Both expression must be of built-in type!",
        getType(cast_to).isBuiltin(), getType(cast_to).isBuiltin());
      assert_true("Both expression must be of built-in type!",
        getType(cast_to).isBuiltinAnd(&isBytes) || getType(cast_to).isBuiltinAnd(&isBytes));
      return addNewProd<CastExpr>(range, cast_to, to_cast, true);
    }

    /// @brief Creates an address of expression.
    /// @param range The range of tokens
    /// @param decl The variable declaration whose address to return
    /// @return AddressOfExpr
    ProdExprToken addAddressOf(TokenRange range, StmtExprToken decl) noexcept
    {
      auto& ref = getExpr(decl);
      if (auto ptr = ref.as<VarDeclExpr>(); ptr)
        return addNewProd<AddressOfExpr>(range,
          ptr->isMut() ? types.addMutPtr(ptr->getType()) : types.addPtr(ptr->getType()), decl);
      if (auto ptr = ref.as<GlobalDeclExpr>(); ptr)
        return addNewProd<AddressOfExpr>(range,
          ptr->isMut() ? types.addMutPtr(ptr->getType()) : types.addPtr(ptr->getType()), decl);

      clt::unreachable("Expected a variable declaration!");
    }
     
    /// @brief Creates a pointer load.
    /// @param range The range of tokens
    /// @param to_cast The expression from which to load (of pointer type)
    /// @return PtrLoadExpr
    ProdExprToken addPtrLoad(TokenRange range, ProdExprToken to_cast) noexcept
    {
      assert_true("Expression must have a pointer type!", getType(to_cast).isAnyPtr()
        && !getType(to_cast).isAnyOpaquePtr());
      auto& ref = getType(to_cast);
      if (auto ptr = ref.as<MutPtrType>(); ptr)
        return addNewProd<PtrLoadExpr>(range, ptr->getPointingTo(), to_cast);
      if (auto ptr = ref.as<PtrType>(); ptr)
        return addNewProd<PtrLoadExpr>(range, ptr->getPointingTo(), to_cast);      
      clt::unreachable("Invalid type!");
    }

    /// @brief Creates a read from a variable.
    /// @param range The range of tokens
    /// @param var_decl The variable declaration from which to read
    /// @return VarReadExpr
    ProdExprToken addVarRead(TokenRange range, StmtExprToken var_decl) noexcept
    {
      assert_true("Expected a VarDeclExpr!", getExpr(var_decl).isVarDecl());
      return addNewProd<VarReadExpr>(range, getExpr(var_decl).getType(), var_decl);
    }
    
    /// @brief Creates a read from a variable.
    /// @param range The range of tokens
    /// @param var_decl The variable declaration from which to read
    /// @return GlobalReadExpr
    ProdExprToken addGlobalRead(TokenRange range, StmtExprToken var_decl) noexcept
    {
      assert_true("Expected a GlobalDeclExpr!", getExpr(var_decl).isGlobalDecl());
      return addNewProd<GlobalReadExpr>(range, getExpr(var_decl).getType(), var_decl);
    }

    ProdExprToken addFnCall() noexcept
    {
      // TODO: add body
      unreachable("NOT IMPLEMENTED!");
    }

    /// @brief Creates a write to a variable
    /// @param range The range of tokens
    /// @param var_decl The variable declaration
    /// @param value The value to write
    /// @return VarWriteExpr
    ProdExprToken addVarWrite(TokenRange range, StmtExprToken var_decl, ProdExprToken value) noexcept
    {
      assert_true("Expected a VarDeclExpr!", getExpr(var_decl).isVarDecl());
      assert_true("Types must match!", getExpr(var_decl).getType() == getTypeToken(value));
      return addNewProd<VarWriteExpr>(range, types.getVoidType(), var_decl, value);
    }
    
    /// @brief Creates a write to a global variable
    /// @param range The range of tokens
    /// @param var_decl The variable declaration
    /// @param value The value to write
    /// @return GlobalWriteExpr
    ProdExprToken addGlobalWrite(TokenRange range, StmtExprToken var_decl, ProdExprToken value) noexcept
    {
      assert_true("Expected a GlobalDeclExpr!", getExpr(var_decl).isGlobalDecl());
      assert_true("Types must match!", getExpr(var_decl).getType() == getTypeToken(value));
      return addNewProd<GlobalWriteExpr>(range, types.getVoidType(), var_decl, value);
    }

    /// @brief Creates a store to a pointer
    /// @param range The range of tokens
    /// @param write_to The pointer to which to write
    /// @param to_write The value to write to that pointer
    /// @return PtrStoreExpr
    ProdExprToken addPtrStore(TokenRange range, ProdExprToken write_to, ProdExprToken to_write) noexcept
    {
      assert_true("Expected a non-opaque pointer to mutable type!", getType(write_to).isMutPtr()
        && !getType(write_to).isAnyOpaquePtr());
      assert_true("Types must match!", getType(write_to).as<MutPtrType>()->getPointingTo() == getTypeToken(to_write));
      return addNewProd<PtrStoreExpr>(range, types.getVoidType(), write_to, to_write);
    }

    /// @brief Creates a move from a variable to another
    /// @param range The range of tokens
    /// @param from The variable to move from
    /// @param to The variable to move to
    /// @return MoveExpr
    ProdExprToken addMove(TokenRange range, StmtExprToken from, StmtExprToken to) noexcept
    {
      assert_true("Expected two local variable declarations!", getExpr(from).isVarDecl(), getExpr(to).isVarDecl());
      return addNewProd<MoveExpr>(range, types.getVoidType(), from, to);
    }
    
    /// @brief Creates a copy from a variable to another
    /// @param range The range of tokens
    /// @param from The variable to copy from
    /// @param to The variable to copy to
    /// @return CopyExpr
    ProdExprToken addCopy(TokenRange range, StmtExprToken from, StmtExprToken to) noexcept
    {
      assert_true("Expected two variable declarations!",
        getExpr(from).isVarDecl() || getExpr(from).isGlobalDecl(),
        getExpr(to).isVarDecl() || getExpr(to).isGlobalDecl());
      return addNewProd<CopyExpr>(range, types.getVoidType(), from, to);
    }
    
    /// @brief Creates a conditional move from a variable to another
    /// @param range The range of tokens
    /// @param from The variable to conditionally move from
    /// @param to The variable to move to
    /// @return CMoveExpr
    ProdExprToken addCMove(TokenRange range, StmtExprToken from, StmtExprToken to) noexcept
    {
      assert_true("Expected two variable declarations!",
        getExpr(from).isVarDecl() || getExpr(from).isGlobalDecl(),
        getExpr(to).isVarDecl() || getExpr(to).isGlobalDecl());
      return addNewProd<CMoveExpr>(range, types.getVoidType(), from, to);
    }

    /// @brief Creates a scope
    /// @param range The range of tokens
    /// @return ScopeExpr
    StmtExprToken addScope(TokenRange range) noexcept
    {
      return addNewStmt<ScopeExpr>(range, types.getVoidType());
    }


    /// @brief Creates a scope
    /// @param range The range of tokens
    /// @param parent The parent of the scope
    /// @return ScopeExpr
    StmtExprToken addScope(TokenRange range, StmtExprToken parent) noexcept
    {
      assert_true("Expected a scope as a parent!", getExpr(parent).isScope());
      return addNewStmt<ScopeExpr>(range, types.getVoidType(), parent);
    }
    
    /// @brief Creates a condition
    /// @param range The range of tokens
    /// @param if_cond The if condition
    /// @param if_stmt The if statement
    /// @param else_stmt The else statement
    /// @return ConditionExpr
    StmtExprToken addCondition(TokenRange range, ProdExprToken if_cond, StmtExprToken if_stmt, OptTok<StmtExprToken> else_stmt) noexcept
    {
      assert_true("Expected bool type!", getType(if_cond).isBuiltinAnd(&isBool));
      return addNewStmt<ConditionExpr>(range, types.getVoidType(), if_cond, if_stmt, else_stmt);
    }

    StmtExprToken addGlobalDecl(TokenRange range, TypeToken type, StringView name, ProdExprToken init, bool is_mut) noexcept
    {
      return addNewStmt<GlobalDeclExpr>(range, type, name, init, is_mut);
    }

    StmtExprToken addVarDecl(TokenRange range, TypeToken type, u32 local_id, StringView name, OptTok<ProdExprToken> init, bool is_mut) noexcept
    {
      return addNewStmt<VarDeclExpr>(range, type, local_id, name, init, is_mut);
    }
  };
}

#endif // !HG_COLT_EXPR_BUFFER
