#include "colt_type.h"

namespace clt::lng
{
  BinarySupport ErrorType::supports(BinaryOp op, const TypeVariant& var) const noexcept
  {
    return ErrorSupport(op, var);
  }

  ConversionSupport ErrorType::castableTo(const TypeVariant& var) const noexcept
  {
    return ErrorCastable(var);
  }
  
  BinarySupport VoidType::supports(BinaryOp op, const TypeVariant& var) const noexcept
  {
    return NoSupport(op, var);
  }
  
  ConversionSupport VoidType::castableTo(const TypeVariant& var) const noexcept
  {
    return NotCastable(var);
  }
  
  BinarySupport OpaquePtrType::supports(BinaryOp op, const TypeVariant& var) const noexcept
  {
    return OpaquePtrSupport(op, var);
  }
  
  ConversionSupport OpaquePtrType::castableTo(const TypeVariant& var) const noexcept
  {
    return NotCastable(var);
  }

  BinarySupport MutOpaquePtrType::supports(BinaryOp op, const TypeVariant& var) const noexcept
  {
    return OpaquePtrSupport(op, var);
  }

  ConversionSupport MutOpaquePtrType::castableTo(const TypeVariant& var) const noexcept
  {
    return NotCastable(var);
  }

  BinarySupport BuiltinType::supports(BinaryOp op, const TypeVariant& var) const noexcept
  {
    return BuiltinSupport(typeID(), op, var);
  }

  ConversionSupport BuiltinType::castableTo(const TypeVariant& var) const noexcept
  {
    return BuiltinCastable(var);
  }
  

  BinarySupport PointerType::supports(BinaryOp op, const TypeVariant& var) const noexcept
  {
    return PtrSupport(*this, op, var);
  }
  
  ConversionSupport PointerType::castableTo(const TypeVariant& var) const noexcept
  {
    return NotCastable(var);
  }

  BinarySupport FnType::supports(BinaryOp op, const TypeVariant& var) const noexcept
  {
    return NoSupport(op, var);
  }

  ConversionSupport FnType::castableTo(const TypeVariant& var) const noexcept
  {
    return NotCastable(var);
  }

}
