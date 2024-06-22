#include "colt_type.h"

namespace clt::lng
{
  BinarySupport ErrorType::supports(BinaryOp op, const TypeVariant& var) const noexcept
  {
    return error_support(op, var);
  }

  ConversionSupport ErrorType::castable_to(const TypeVariant& var) const noexcept
  {
    return error_castable(var);
  }
  
  BinarySupport VoidType::supports(BinaryOp op, const TypeVariant& var) const noexcept
  {
    return no_support(op, var);
  }
  
  ConversionSupport VoidType::castable_to(const TypeVariant& var) const noexcept
  {
    return not_castable(var);
  }
  
  BinarySupport OpaquePtrType::supports(BinaryOp op, const TypeVariant& var) const noexcept
  {
    return opaque_ptr_support(op, var);
  }
  
  ConversionSupport OpaquePtrType::castable_to(const TypeVariant& var) const noexcept
  {
    return not_castable(var);
  }

  BinarySupport MutOpaquePtrType::supports(BinaryOp op, const TypeVariant& var) const noexcept
  {
    return opaque_ptr_support(op, var);
  }

  ConversionSupport MutOpaquePtrType::castable_to(const TypeVariant& var) const noexcept
  {
    return not_castable(var);
  }

  BinarySupport BuiltinType::supports(BinaryOp op, const TypeVariant& var) const noexcept
  {
    return builtin_support(type_id(), op, var);
  }

  ConversionSupport BuiltinType::castable_to(const TypeVariant& var) const noexcept
  {
    return builtin_castable(var);
  }
  

  BinarySupport PointerType::supports(BinaryOp op, const TypeVariant& var) const noexcept
  {
    return ptr_support(*this, op, var);
  }
  
  ConversionSupport PointerType::castable_to(const TypeVariant& var) const noexcept
  {
    return not_castable(var);
  }

  BinarySupport FnType::supports(BinaryOp op, const TypeVariant& var) const noexcept
  {
    return no_support(op, var);
  }

  ConversionSupport FnType::castable_to(const TypeVariant& var) const noexcept
  {
    return not_castable(var);
  }

}
