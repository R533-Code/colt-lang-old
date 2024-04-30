/*****************************************************************//**
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
  UnarySupport ErrorSupport([[maybe_unused]] UnaryOp op) noexcept
  {
    return UnarySupport::BUILTIN;
  }
  
  UnarySupport NoSupport([[maybe_unused]] UnaryOp op) noexcept
  {
    return UnarySupport::INVALID;
  }
  
  UnarySupport PtrSupport([[maybe_unused]] UnaryOp op) noexcept
  {
    return UnarySupport::INVALID;
  }
  
  UnarySupport BoolSupport(UnaryOp op) noexcept
  {
    using enum UnaryOp;
    if (op == OP_BOOL_NOT)
      return UnarySupport::BUILTIN;
    return UnarySupport::INVALID;
  }
  
  UnarySupport SIntSupport(UnaryOp op) noexcept
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
  
  UnarySupport UIntSupport(UnaryOp op) noexcept
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
  
  UnarySupport FPSupport(UnaryOp op) noexcept
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
  
  UnarySupport BytesSupport(UnaryOp op) noexcept
  {
    using enum UnaryOp;
    if (op == OP_BIT_NOT)
      return UnarySupport::BUILTIN;
    return UnarySupport::INVALID;
  }
  
  UnarySupport BuiltinSupport(BuiltinID ID, UnaryOp op) noexcept
  {
    using enum BuiltinID;
   
    switch_no_default(ID)
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
  
  BinarySupport ErrorSupport([[maybe_unused]] BinaryOp op, [[maybe_unused]] const TypeVariant& var) noexcept
  {
    return BinarySupport::BUILTIN;
  }  

  BinarySupport NoSupport([[maybe_unused]] BinaryOp op, [[maybe_unused]] const TypeVariant& var) noexcept
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
      if (auto ptr = rhs.as<PointerType>(); ptr && ptr->getPointingTo() == lhs.getPointingTo())
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
      if (auto ptr = var.as<BuiltinType>(); ptr && ptr->typeID() == lhs)
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
      if (auto ptr = var.as<BuiltinType>(); ptr && ptr->typeID() == lhs)
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
      if (auto ptr = var.as<BuiltinType>(); ptr && ptr->typeID() == lhs)
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
      if (auto ptr = var.as<BuiltinType>(); ptr && ptr->typeID() == lhs)
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

  ConversionSupport ErrorCastable([[maybe_unused]] const TypeVariant& var) noexcept
  {
    return ConversionSupport::BUILTIN;
  }
  
  ConversionSupport NotCastable([[maybe_unused]] const TypeVariant& var) noexcept
  {
    return ConversionSupport::INVALID;
  }
  
  ConversionSupport BuiltinCastable(const TypeVariant& var) noexcept
  {
    if (var.isBuiltin())
      return ConversionSupport::BUILTIN;
    return ConversionSupport::INVALID;
  }
}