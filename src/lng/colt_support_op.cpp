#include "colt_support_op.h"
#include "colt_type.h"

namespace clt::lng
{
  BinarySupport ErrorSupport([[maybe_unused]] BinaryOp op, const TypeVariant& var) noexcept
  {
    return BinarySupport::BUILTIN;
  }
  
  BinarySupport NoSupport([[maybe_unused]] BinaryOp op, const TypeVariant& var) noexcept
  {
    return BinarySupport::INVALID_OP;
  }

  BinarySupport OpaquePtrSupport(BinaryOp op, const TypeVariant& rhs) noexcept
  {
    switch (op)
    {
    case clt::lng::BinaryOp::OP_LESS:
    case clt::lng::BinaryOp::OP_LESS_EQUAL:
    case clt::lng::BinaryOp::OP_GREAT:
    case clt::lng::BinaryOp::OP_GREAT_EQUAL:
    case clt::lng::BinaryOp::OP_NOT_EQUAL:
    case clt::lng::BinaryOp::OP_EQUAL:
      return rhs.isAnyOpaquePtr() ? BinarySupport::BUILTIN : BinarySupport::INVALID_TYPE;
    default:
      return BinarySupport::INVALID_OP;
    }
  }
  
  BinarySupport PtrSupport(const PointerType& lhs, [[maybe_unused]] BinaryOp op, const TypeVariant& rhs) noexcept
  {
    using enum BinaryOp;
    
    switch (op)
    {
    case OP_SUM:
    case OP_SUB:
      if (!rhs.isBuiltinAnd(&isIntegral))
        return BinarySupport::INVALID_TYPE;
      return BinarySupport::BUILTIN;
    case OP_LESS:
    case OP_LESS_EQUAL:
    case OP_GREAT:
    case OP_GREAT_EQUAL:
    case OP_NOT_EQUAL:
    case OP_EQUAL:
      if (auto ptr = rhs.getType<PointerType>(); ptr && ptr->getPointingTo() == lhs.getPointingTo())
        return BinarySupport::BUILTIN;
      return BinarySupport::INVALID_TYPE;
    default:
      return BinarySupport::INVALID_OP;
    }
  }
  
  BinarySupport BoolSupport(BinaryOp op, const TypeVariant& var) noexcept
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
      return var.isBuiltinAnd(&isBool) ? BUILTIN : INVALID_TYPE;
    default:
      return INVALID_OP;
    }
  }
  
  BinarySupport SIntSupport(BuiltinID lhs, BinaryOp op, const TypeVariant& var) noexcept
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
      if (auto ptr = var.getType<BuiltinType>(); ptr && ptr->typeID() == lhs)
        return BinarySupport::BUILTIN;
      return BinarySupport::INVALID_TYPE;
    default:
      return BinarySupport::INVALID_OP;
    }
  }
  
  BinarySupport UIntSupport(BuiltinID lhs, BinaryOp op, const TypeVariant& var) noexcept
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
      if (auto ptr = var.getType<BuiltinType>(); ptr && ptr->typeID() == lhs)
        return BinarySupport::BUILTIN;
      return BinarySupport::INVALID_TYPE;
    default:
      return BinarySupport::INVALID_OP;
    }
  }
  
  BinarySupport FPSupport(BuiltinID lhs, BinaryOp op, const TypeVariant& var) noexcept
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
      if (auto ptr = var.getType<BuiltinType>(); ptr && ptr->typeID() == lhs)
        return BinarySupport::BUILTIN;
      return BinarySupport::INVALID_TYPE;
    default:
      return BinarySupport::INVALID_OP;
    }
  }
  
  BinarySupport BytesSupport(BuiltinID lhs, BinaryOp op, const TypeVariant& var) noexcept
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
      if (auto ptr = var.getType<BuiltinType>(); ptr && ptr->typeID() == lhs)
        return BinarySupport::BUILTIN;
      return BinarySupport::INVALID_TYPE;
    default:
      return BinarySupport::INVALID_OP;
    }
  }
  
  BinarySupport BuiltinSupport(BuiltinID ID, BinaryOp op, const TypeVariant& var) noexcept
  {
    using enum BuiltinID;
    
    switch_no_default(ID)
    {
    case BOOL:
      return BoolSupport(op, var);
    case CHAR:
      return BinarySupport::INVALID_OP;
    case U8:
    case U16:
    case U32:
    case U64:
      return UIntSupport(ID, op, var);
    case I8:
    case I16:
    case I32:
    case I64:
      return SIntSupport(ID, op, var);
    case F32:
    case F64:
      return FPSupport(ID, op, var);
    case BYTE:
    case WORD:
    case DWORD:
    case QWORD:
      return BytesSupport(ID, op, var);
    }
  }
}