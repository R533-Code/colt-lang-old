#ifndef HG_COLT_EXPR_BUFFER
#define HG_COLT_EXPR_BUFFER

#include "ast/colt_expr.h"

namespace clt::lng
{
  /// @brief Variant of expression that produce a value
  class ProdExprVariant
  {
    MAKE_UNION_AND_GET_MEMBER(COLTC_PROD_EXPR_LIST);

  public:
    template<ProducerExpr Type, typename... Args>
    /// @brief Constructor
    /// @param args... The arguments to forward to the constructor
    constexpr ProdExprVariant(std::type_identity<Type>, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<Type, Args...>)
      : _mono_state_(construct<Type>(&getUnionMember<Type>(), std::forward<Args>(args)...))
    {
      // The ugly code above is used to keep the constructor constexpr
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
    constexpr T* getExpr() noexcept
    {
      if (classof() != TypeToExprID<T>())
        return nullptr;
      return &getUnionMember<T>();
    }
    
    template<ProducerExpr T>
    /// @brief Downcasts the variant to 'T'
    /// @return nullptr if type does not match else pointer to the type
    constexpr const T* getExpr() const noexcept
    {
      if (classof() != TypeToExprID<T>())
        return nullptr;
      return &getUnionMember<T>();
    }
  };

  /// @brief Class responsible of the lifetimes of all expressions
  class ExprBuffer
  {
    /// @brief The buffer of types
    TypeBuffer& types;
    /// @brief The list of producer expression
    FlatList<ProdExprVariant, 512> prod_expr;

    /// @brief Returns the next ProdExprToken.
    /// A push_back to prod_expr must follow this call.
    /// @return The next ProdExprToken
    ProdExprToken getNextProd() noexcept
    {
      assert_true("Integer overflow!", prod_expr.size() <= std::numeric_limits<u32>::max());
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

  public:
    /// @brief Returns the expression represented by 'prod'
    /// @param prod The producer expression token
    /// @return Reference to the expression represented by 'prod'
    ProdExprVariant& getExpr(ProdExprToken prod) noexcept { return prod_expr[prod]; }
    /// @brief Returns the expression represented by 'prod'
    /// @param prod The producer expression token
    /// @return Reference to the expression represented by 'prod'
    const ProdExprVariant& getExpr(ProdExprToken prod) const noexcept { return prod_expr[prod]; }

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
    ProdExprToken addProdError(TokenRange range) noexcept
    {
      return addNewProd<ErrorExpr>(range, types.getErrorType());
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
      assert_true("Operator must not be an assignment operator!", FamilyOf(op) != OpFamily::ASSIGNMENT);
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
      //TODO: add body
      unreachable("NOT IMPLEMENTED!");
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
      if (auto ptr = ref.getType<MutPtrType>(); ptr)
        return addNewProd<PtrLoadExpr>(range, ptr->getPointingTo(), to_cast);
      if (auto ptr = ref.getType<PtrType>(); ptr)
        return addNewProd<PtrLoadExpr>(range, ptr->getPointingTo(), to_cast);      
      clt::unreachable("Invalid type!");
    }

    /// @brief Creates a read from a variable.
    /// @param range The range of tokens
    /// @param var_decl The variable declaration from which to read
    /// @return VarReadExpr
    ProdExprToken addVarRead(TokenRange range, StmtExprToken var_decl) noexcept
    {
      //TODO: add body
      unreachable("NOT IMPLEMENTED!");
    }
    
    /// @brief Creates a read from a variable.
    /// @param range The range of tokens
    /// @param var_decl The variable declaration from which to read
    /// @return VarReadExpr
    ProdExprToken addGlobalRead(TokenRange range, StmtExprToken var_decl) noexcept
    {
      //TODO: add body
      unreachable("NOT IMPLEMENTED!");
    }

    ProdExprToken addFnCall() noexcept
    {
      // TODO: add body
      unreachable("NOT IMPLEMENTED!");
    }
  };
}

#endif // !HG_COLT_EXPR_BUFFER
