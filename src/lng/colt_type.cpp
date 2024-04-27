#include "colt_type.h"

namespace clt::lng
{
  BinarySupport ErrorType::supports(BinaryOp op, const TypeVariant& var) const noexcept
  {
    return ErrorSupport(op, var);
  }
  
  BinarySupport VoidType::supports(BinaryOp op, const TypeVariant& var) const noexcept
  {
    return NoSupport(op, var);
  }
  
  BinarySupport OpaquePtrType::supports(BinaryOp op, const TypeVariant& var) const noexcept
  {
    return OpaquePtrSupport(op, var);
  }
  
  BinarySupport MutOpaquePtrType::supports(BinaryOp op, const TypeVariant& var) const noexcept
  {
    return OpaquePtrSupport(op, var);
  }

  BinarySupport BuiltinType::supports(BinaryOp op, const TypeVariant& var) const noexcept
  {
    return BuiltinSupport(typeID(), op, var);
  }

  BinarySupport PointerType::supports(BinaryOp op, const TypeVariant& var) const noexcept
  {
    return PtrSupport(*this, op, var);
  }

  BinarySupport FnType::supports(BinaryOp op, const TypeVariant& var) const noexcept
  {
    return NoSupport(op, var);
  }

}
