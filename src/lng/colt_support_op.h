#ifndef HG_COLT_SUPPORT_OP
#define HG_COLT_SUPPORT_OP

#include "lex/colt_operators.h"
#include "colt_builtin_id.h"

DECLARE_ENUM_WITH_TYPE(u8, clt::lng, UnarySupport,
  BUILTIN, INVALID);

DECLARE_ENUM_WITH_TYPE(u8, clt::lng, BinarySupport,
  BUILTIN, INVALID);

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
}

#endif // !HG_COLT_SUPPORT_OP