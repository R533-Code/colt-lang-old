/*****************************************************************/ /**
 * @file   colt_support_op.cpp
 * @brief  Contains the implementation of `colt_support_op.h`.
 *
 * @author RPC
 * @date   April 2024
 *********************************************************************/
#include "colt_support_op.h"
#include "colt_type.h"

namespace clt::lng
{
  UnarySupport error_support([[maybe_unused]] UnaryOp op) noexcept
  {
    return UnarySupport::BUILTIN;
  }

  UnarySupport no_support([[maybe_unused]] UnaryOp op) noexcept
  {
    return UnarySupport::INVALID;
  }

  UnarySupport ptr_support([[maybe_unused]] UnaryOp op) noexcept
  {
    return UnarySupport::INVALID;
  }

  UnarySupport bool_support(UnaryOp op) noexcept
  {
    using enum UnaryOp;
    if (op == OP_BOOL_NOT)
      return UnarySupport::BUILTIN;
    return UnarySupport::INVALID;
  }

  UnarySupport sint_support(UnaryOp op) noexcept
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

  UnarySupport uint_support(UnaryOp op) noexcept
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

  UnarySupport fp_support(UnaryOp op) noexcept
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

  UnarySupport bytes_support(UnaryOp op) noexcept
  {
    using enum UnaryOp;
    if (op == OP_BIT_NOT)
      return UnarySupport::BUILTIN;
    return UnarySupport::INVALID;
  }

  UnarySupport builtin_support(BuiltinID ID, UnaryOp op) noexcept
  {
    using enum BuiltinID;

    switch_no_default(ID)
    {
    case BOOL:
      return bool_support(op);
    case CHAR:
      return UnarySupport::INVALID;
    case U8:
    case U16:
    case U32:
    case U64:
      return uint_support(op);
    case I8:
    case I16:
    case I32:
    case I64:
      return sint_support(op);
    case F32:
    case F64:
      return fp_support(op);
    case BYTE:
    case WORD:
    case DWORD:
    case QWORD:
      return bytes_support(op);
    }
  }

  BinarySupport error_support(
      [[maybe_unused]] BinaryOp op, [[maybe_unused]] const TypeVariant& var) noexcept
  {
    return BinarySupport::BUILTIN;
  }

  BinarySupport no_support(
      [[maybe_unused]] BinaryOp op, [[maybe_unused]] const TypeVariant& var) noexcept
  {
    return BinarySupport::INVALID_OP;
  }

  BinarySupport opaque_ptr_support(BinaryOp op, const TypeVariant& rhs) noexcept
  {
    switch (op)
    {
    case clt::lng::BinaryOp::OP_LESS:
    case clt::lng::BinaryOp::OP_LESS_EQUAL:
    case clt::lng::BinaryOp::OP_GREAT:
    case clt::lng::BinaryOp::OP_GREAT_EQUAL:
    case clt::lng::BinaryOp::OP_NOT_EQUAL:
    case clt::lng::BinaryOp::OP_EQUAL:
      return rhs.is_any_opaque_ptr() ? BinarySupport::BUILTIN
                                     : BinarySupport::INVALID_TYPE;
    default:
      return BinarySupport::INVALID_OP;
    }
  }

  BinarySupport ptr_support(
      const PointerType& lhs, [[maybe_unused]] BinaryOp op,
      const TypeVariant& rhs) noexcept
  {
    using enum BinaryOp;

    switch (op)
    {
    case OP_SUM:
    case OP_SUB:
      if (!rhs.is_builtin_and(&is_integral))
        return BinarySupport::INVALID_TYPE;
      return BinarySupport::BUILTIN;
    case OP_LESS:
    case OP_LESS_EQUAL:
    case OP_GREAT:
    case OP_GREAT_EQUAL:
    case OP_NOT_EQUAL:
    case OP_EQUAL:
      if (auto ptr = rhs.as<PointerType>();
          ptr && ptr->pointing_to() == lhs.pointing_to())
        return BinarySupport::BUILTIN;
      return BinarySupport::INVALID_TYPE;
    default:
      return BinarySupport::INVALID_OP;
    }
  }

  BinarySupport bool_support(BinaryOp op, const TypeVariant& var) noexcept
  {
    using enum BinaryOp;
    using enum BinarySupport;
    switch (op)
    {
    case OP_BIT_AND:
    case OP_BIT_OR:
    case OP_BIT_XOR:
    case OP_BOOL_AND:
    case OP_BOOL_OR:
    case OP_NOT_EQUAL:
    case OP_EQUAL:
      return var.is_builtin_and(&is_bool) ? BUILTIN : INVALID_TYPE;
    default:
      return INVALID_OP;
    }
  }

  BinarySupport sint_support(
      BuiltinID lhs, BinaryOp op, const TypeVariant& var) noexcept
  {
    using enum BinaryOp;
    switch (op)
    {
    case OP_SUM:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
    case OP_MOD:
    case OP_BIT_AND:
    case OP_BIT_OR:
    case OP_BIT_XOR:
    case OP_BIT_LSHIFT:
    case OP_BIT_RSHIFT:
    case OP_LESS:
    case OP_LESS_EQUAL:
    case OP_GREAT:
    case OP_GREAT_EQUAL:
    case OP_NOT_EQUAL:
    case OP_EQUAL:
      if (auto ptr = var.as<BuiltinType>(); ptr && ptr->type_id() == lhs)
        return BinarySupport::BUILTIN;
      return BinarySupport::INVALID_TYPE;
    default:
      return BinarySupport::INVALID_OP;
    }
  }

  BinarySupport uint_support(
      BuiltinID lhs, BinaryOp op, const TypeVariant& var) noexcept
  {
    using enum BinaryOp;
    switch (op)
    {
    case OP_SUM:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
    case OP_MOD:
    case OP_BIT_AND:
    case OP_BIT_OR:
    case OP_BIT_XOR:
    case OP_BIT_LSHIFT:
    case OP_BIT_RSHIFT:
    case OP_LESS:
    case OP_LESS_EQUAL:
    case OP_GREAT:
    case OP_GREAT_EQUAL:
    case OP_NOT_EQUAL:
    case OP_EQUAL:
      if (auto ptr = var.as<BuiltinType>(); ptr && ptr->type_id() == lhs)
        return BinarySupport::BUILTIN;
      return BinarySupport::INVALID_TYPE;
    default:
      return BinarySupport::INVALID_OP;
    }
  }

  BinarySupport fp_support(
      BuiltinID lhs, BinaryOp op, const TypeVariant& var) noexcept
  {
    using enum BinaryOp;
    switch (op)
    {
    case OP_SUM:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
    case OP_MOD:
    case OP_LESS:
    case OP_LESS_EQUAL:
    case OP_GREAT:
    case OP_GREAT_EQUAL:
    case OP_NOT_EQUAL:
    case OP_EQUAL:
      if (auto ptr = var.as<BuiltinType>(); ptr && ptr->type_id() == lhs)
        return BinarySupport::BUILTIN;
      return BinarySupport::INVALID_TYPE;
    default:
      return BinarySupport::INVALID_OP;
    }
  }

  BinarySupport bytes_support(
      BuiltinID lhs, BinaryOp op, const TypeVariant& var) noexcept
  {
    using enum BinaryOp;
    switch (op)
    {
    case OP_SUM:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
    case OP_MOD:
    case OP_BIT_AND:
    case OP_BIT_OR:
    case OP_BIT_XOR:
    case OP_BIT_LSHIFT:
    case OP_BIT_RSHIFT:
    case OP_LESS:
    case OP_LESS_EQUAL:
    case OP_GREAT:
    case OP_GREAT_EQUAL:
    case OP_NOT_EQUAL:
    case OP_EQUAL:
      if (auto ptr = var.as<BuiltinType>(); ptr && ptr->type_id() == lhs)
        return BinarySupport::BUILTIN;
      return BinarySupport::INVALID_TYPE;
    default:
      return BinarySupport::INVALID_OP;
    }
  }

  BinarySupport builtin_support(
      BuiltinID ID, BinaryOp op, const TypeVariant& var) noexcept
  {
    using enum BuiltinID;

    switch_no_default(ID)
    {
    case BOOL:
      return bool_support(op, var);
    case CHAR:
      return BinarySupport::INVALID_OP;
    case U8:
    case U16:
    case U32:
    case U64:
      return uint_support(ID, op, var);
    case I8:
    case I16:
    case I32:
    case I64:
      return sint_support(ID, op, var);
    case F32:
    case F64:
      return fp_support(ID, op, var);
    case BYTE:
    case WORD:
    case DWORD:
    case QWORD:
      return bytes_support(ID, op, var);
    }
  }

  ConversionSupport error_castable([[maybe_unused]] const TypeVariant& var) noexcept
  {
    return ConversionSupport::BUILTIN;
  }

  ConversionSupport not_castable([[maybe_unused]] const TypeVariant& var) noexcept
  {
    return ConversionSupport::INVALID;
  }

  ConversionSupport builtin_castable(const TypeVariant& var) noexcept
  {
    if (var.is_builtin())
      return ConversionSupport::BUILTIN;
    return ConversionSupport::INVALID;
  }
} // namespace clt::lng