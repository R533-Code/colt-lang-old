#ifndef HG_COLT_SUPPORT_OP
#define HG_COLT_SUPPORT_OP

#include "lex/colt_operators.h"
#include "colt_builtin_id.h"

DECLARE_ENUM_WITH_TYPE(u8, clt::lng, UnarySupport,
  // Built-in operator
  BUILTIN,
  // Invalid operator
  INVALID
);

DECLARE_ENUM_WITH_TYPE(u8, clt::lng, BinarySupport,
  // Both operands are of type BuiltinType
  BUILTIN,
  // Invalid operator
  INVALID_OP,
  // Invalid right hand side
  INVALID_TYPE
);

namespace clt::lng
{
  /// @brief Check which unary operators are supported by an error type
  /// @param op The unary operator whose support to check
  /// @return BUILTIN
  constexpr UnarySupport ErrorSupport([[maybe_unused]] UnaryOp op) noexcept
  {
    return UnarySupport::BUILTIN;
  }
  
  /// @brief Check which unary operators are supported by an error type
  /// @param op The unary operator whose support to check
  /// @return INVALID
  constexpr UnarySupport NoSupport([[maybe_unused]] UnaryOp op) noexcept
  {
    return UnarySupport::INVALID;
  }

  /// @brief Check which unary operators are supported by a pointer type
  /// @param op The unary operator whose support to check
  /// @return INVALID
  constexpr UnarySupport PtrSupport([[maybe_unused]] UnaryOp op) noexcept
  {
    return UnarySupport::INVALID;
  }

  /// @brief Check which unary operators are supported by a BOOL type
  /// @param op The unary operator whose support to check
  /// @return INVALID or BUILTIN
  constexpr UnarySupport BoolSupport(UnaryOp op) noexcept
  {
    using enum UnaryOp;
    if (op == OP_BOOL_NOT)
      return UnarySupport::BUILTIN;
    return UnarySupport::INVALID;
  }

  /// @brief Check which unary operators are supported by a signed int type
  /// @param op The unary operator whose support to check
  /// @return INVALID or BUILTIN
  constexpr UnarySupport SIntSupport(UnaryOp op) noexcept
  {
    using enum UnaryOp;
    switch (op)
    {
    case OP_BIT_NOT:
    case OP_NEGATE:
    case OP_INC:
    case OP_DEC:
      return UnarySupport::BUILTIN;
    default:
      return UnarySupport::INVALID;
    }
  }
  
  /// @brief Check which unary operators are supported by an unsigned int type
  /// @param op The unary operator whose support to check
  /// @return INVALID or BUILTIN
  constexpr UnarySupport UIntSupport(UnaryOp op) noexcept
  {
    using enum UnaryOp;
    switch (op)
    {
    case OP_BIT_NOT:
    case OP_INC:
    case OP_DEC:
      return UnarySupport::BUILTIN;
    default:
      return UnarySupport::INVALID;
    }
  }
  
  /// @brief Check which unary operators are supported by a floating point type
  /// @param op The unary operator whose support to check
  /// @return INVALID or BUILTIN
  constexpr UnarySupport FPSupport(UnaryOp op) noexcept
  {
    using enum UnaryOp;
    switch (op)
    {
    case OP_INC:
    case OP_DEC:
    case OP_NEGATE:
      return UnarySupport::BUILTIN;
    default:
      return UnarySupport::INVALID;
    }
  }
  
  /// @brief Check which unary operators are supported by byte type
  /// @param op The unary operator whose support to check
  /// @return INVALID or BUILTIN
  constexpr UnarySupport BytesSupport(UnaryOp op) noexcept
  {
    using enum UnaryOp;
    if (op == OP_BIT_NOT)
      return UnarySupport::BUILTIN;
    return UnarySupport::INVALID;
  }

  /// @brief Check if the type with ID 'ID' supports 'op'
  /// @param ID The type ID
  /// @param op The operator whose support to check
  /// @return UnarySupport
  constexpr UnarySupport BuiltinSupport(BuiltinID ID, UnaryOp op) noexcept
  {
    using enum BuiltinID;
    switch_no_default (ID)
    {
    case BOOL:
      return BoolSupport(op);
    case CHAR:
      return UnarySupport::INVALID;
    case U8:
    case U16:
    case U32:
    case U64:
      return UIntSupport(op);
    case I8:
    case I16:
    case I32:
    case I64:
      return SIntSupport(op);
    case F32:
    case F64:
      return FPSupport(op);
    case BYTE:
    case WORD:
    case DWORD:
    case QWORD:
      return BytesSupport(op);
    }
  }

  class TypeVariant;
  class PointerType;

  /// @brief Check which unary operators are supported by an error type
  /// @param op The binary operator whose support to check
  /// @param var The right hand side of the operator
  /// @return BUILTIN
  BinarySupport ErrorSupport([[maybe_unused]] BinaryOp op, const TypeVariant& var) noexcept;

  /// @brief Check which unary operators are supported by an error type
  /// @param op The binary operator whose support to check
  /// @param var The right hand side of the operator
  /// @return INVALID
  BinarySupport NoSupport([[maybe_unused]] BinaryOp op, const TypeVariant& var) noexcept;

  /// @brief Check which unary operators are supported by a pointer type
  /// @param op The unary operator whose support to check
  /// @param rhs The right hand side of the operator
  /// @return BinarySupport
  BinarySupport OpaquePtrSupport([[maybe_unused]] BinaryOp op, const TypeVariant& rhs) noexcept;

  /// @brief Check which unary operators are supported by a pointer type
  /// @param lhs The left hand side
  /// @param op The unary operator whose support to check
  /// @param rhs The right hand side of the operator
  /// @return BinarySupport
  BinarySupport PtrSupport(const PointerType& tkn, [[maybe_unused]] BinaryOp op, const TypeVariant& var) noexcept;

  /// @brief Check which unary operators are supported by a BOOL type
  /// @param op The unary operator whose support to check
  /// @param var The right hand side of the operator
  /// @return INVALID or BUILTIN
  BinarySupport BoolSupport(BinaryOp op, const TypeVariant& var) noexcept;

  /// @brief Check which unary operators are supported by a signed int type
  /// @param lhs The left hand side
  /// @param op The unary operator whose support to check
  /// @param var The right hand side of the operator
  /// @return INVALID or BUILTIN
  BinarySupport SIntSupport(BuiltinID lhs, BinaryOp op, const TypeVariant& var) noexcept;

  /// @brief Check which unary operators are supported by an unsigned int type
  /// @param lhs The left hand side
  /// @param op The unary operator whose support to check
  /// @param var The right hand side of the operator
  /// @return INVALID or BUILTIN
  BinarySupport UIntSupport(BuiltinID lhs, BinaryOp op, const TypeVariant& var) noexcept;

  /// @brief Check which unary operators are supported by a floating point type
  /// @param lhs The left hand side
  /// @param op The unary operator whose support to check
  /// @param var The right hand side of the operator
  /// @return INVALID or BUILTIN
  BinarySupport FPSupport(BuiltinID lhs, BinaryOp op, const TypeVariant& var) noexcept;

  /// @brief Check which unary operators are supported by byte type
  /// @param lhs The left hand side
  /// @param op The unary operator whose support to check
  /// @param var The right hand side of the operator
  /// @return INVALID or BUILTIN
  BinarySupport BytesSupport(BuiltinID lhs, BinaryOp op, const TypeVariant& var) noexcept;

  /// @brief Check if the type with ID 'ID' supports 'op'
  /// @param ID The type ID
  /// @param op The operator whose support to check
  /// @param var The right hand side of the operator
  /// @return UnarySupport
  BinarySupport BuiltinSupport(BuiltinID ID, BinaryOp op, const TypeVariant& var) noexcept;  
}

#endif // !HG_COLT_SUPPORT_OP