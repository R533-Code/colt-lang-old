/*****************************************************************//**
 * @file   colt_support_op.h
 * @brief  Contains enum and functions to check if a Colt type
 * supports an operator or a cast.
 * 
 * @author RPC
 * @date   April 2024
 *********************************************************************/
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

DECLARE_ENUM_WITH_TYPE(u8, clt::lng, ConversionSupport,
  // Built-in conversion
  BUILTIN,
  // Invalid conversion
  INVALID
);

namespace clt::lng
{
  class TypeVariant;
  class PointerType;

  /************* UNARY *************/

  /// @brief Check which unary operators are supported by an error type
  /// @param op The unary operator whose support to check
  /// @return BUILTIN
  UnarySupport ErrorSupport([[maybe_unused]] UnaryOp op) noexcept;  
  
  /// @brief Check which unary operators are supported by an error type
  /// @param op The unary operator whose support to check
  /// @return INVALID
  UnarySupport NoSupport([[maybe_unused]] UnaryOp op) noexcept;  

  /// @brief Check which unary operators are supported by a pointer type
  /// @param op The unary operator whose support to check
  /// @return INVALID
  UnarySupport PtrSupport([[maybe_unused]] UnaryOp op) noexcept;  

  /// @brief Check which unary operators are supported by a BOOL type
  /// @param op The unary operator whose support to check
  /// @return INVALID or BUILTIN
  UnarySupport BoolSupport(UnaryOp op) noexcept;

  /// @brief Check which unary operators are supported by a signed int type
  /// @param op The unary operator whose support to check
  /// @return INVALID or BUILTIN
  UnarySupport SIntSupport(UnaryOp op) noexcept;
  
  /// @brief Check which unary operators are supported by an unsigned int type
  /// @param op The unary operator whose support to check
  /// @return INVALID or BUILTIN
  UnarySupport UIntSupport(UnaryOp op) noexcept;
  
  /// @brief Check which unary operators are supported by a floating point type
  /// @param op The unary operator whose support to check
  /// @return INVALID or BUILTIN
  UnarySupport FPSupport(UnaryOp op) noexcept;
  
  /// @brief Check which unary operators are supported by byte type
  /// @param op The unary operator whose support to check
  /// @return INVALID or BUILTIN
  UnarySupport BytesSupport(UnaryOp op) noexcept;  

  /// @brief Check if the type with ID 'ID' supports 'op'
  /// @param ID The type ID
  /// @param op The operator whose support to check
  /// @return UnarySupport
  UnarySupport BuiltinSupport(BuiltinID ID, UnaryOp op) noexcept;  

  /************* BINARY *************/

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

  /************* CONVERSIONS *************/

  /// @brief Check if an error type is castable to another type
  /// @param var The type to cast to
  /// @return BUILTIN
  ConversionSupport ErrorCastable([[maybe_unused]] const TypeVariant& var) noexcept;

  /// @brief Always return INVALID
  /// @param var The type to cast to (unused)
  /// @return INVALID
  ConversionSupport NotCastable([[maybe_unused]] const TypeVariant& var) noexcept;

  /// @brief Check if a type is castable to another type
  /// @param var The type to cast to
  /// @return BUILTIN or INVALID
  ConversionSupport BuiltinCastable(const TypeVariant& var) noexcept;
}

#endif // !HG_COLT_SUPPORT_OP