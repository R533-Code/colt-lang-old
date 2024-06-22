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
  UnarySupport error_support([[maybe_unused]] UnaryOp op) noexcept;  
  
  /// @brief Check which unary operators are supported by an error type
  /// @param op The unary operator whose support to check
  /// @return INVALID
  UnarySupport no_support([[maybe_unused]] UnaryOp op) noexcept;  

  /// @brief Check which unary operators are supported by a pointer type
  /// @param op The unary operator whose support to check
  /// @return INVALID
  UnarySupport ptr_support([[maybe_unused]] UnaryOp op) noexcept;  

  /// @brief Check which unary operators are supported by a BOOL type
  /// @param op The unary operator whose support to check
  /// @return INVALID or BUILTIN
  UnarySupport bool_support(UnaryOp op) noexcept;

  /// @brief Check which unary operators are supported by a signed int type
  /// @param op The unary operator whose support to check
  /// @return INVALID or BUILTIN
  UnarySupport sint_support(UnaryOp op) noexcept;
  
  /// @brief Check which unary operators are supported by an unsigned int type
  /// @param op The unary operator whose support to check
  /// @return INVALID or BUILTIN
  UnarySupport uint_support(UnaryOp op) noexcept;
  
  /// @brief Check which unary operators are supported by a floating point type
  /// @param op The unary operator whose support to check
  /// @return INVALID or BUILTIN
  UnarySupport fp_support(UnaryOp op) noexcept;
  
  /// @brief Check which unary operators are supported by byte type
  /// @param op The unary operator whose support to check
  /// @return INVALID or BUILTIN
  UnarySupport bytes_support(UnaryOp op) noexcept;  

  /// @brief Check if the type with ID 'ID' supports 'op'
  /// @param ID The type ID
  /// @param op The operator whose support to check
  /// @return UnarySupport
  UnarySupport builtin_support(BuiltinID ID, UnaryOp op) noexcept;  

  /************* BINARY *************/

  /// @brief Check which unary operators are supported by an error type
  /// @param op The binary operator whose support to check
  /// @param var The right hand side of the operator
  /// @return BUILTIN
  BinarySupport error_support([[maybe_unused]] BinaryOp op, const TypeVariant& var) noexcept;

  /// @brief Check which unary operators are supported by an error type
  /// @param op The binary operator whose support to check
  /// @param var The right hand side of the operator
  /// @return INVALID
  BinarySupport no_support([[maybe_unused]] BinaryOp op, const TypeVariant& var) noexcept;

  /// @brief Check which unary operators are supported by a pointer type
  /// @param op The unary operator whose support to check
  /// @param rhs The right hand side of the operator
  /// @return BinarySupport
  BinarySupport opaque_ptr_support([[maybe_unused]] BinaryOp op, const TypeVariant& rhs) noexcept;

  /// @brief Check which unary operators are supported by a pointer type
  /// @param lhs The left hand side
  /// @param op The unary operator whose support to check
  /// @param rhs The right hand side of the operator
  /// @return BinarySupport
  BinarySupport ptr_support(const PointerType& tkn, [[maybe_unused]] BinaryOp op, const TypeVariant& var) noexcept;

  /// @brief Check which unary operators are supported by a BOOL type
  /// @param op The unary operator whose support to check
  /// @param var The right hand side of the operator
  /// @return INVALID or BUILTIN
  BinarySupport bool_support(BinaryOp op, const TypeVariant& var) noexcept;

  /// @brief Check which unary operators are supported by a signed int type
  /// @param lhs The left hand side
  /// @param op The unary operator whose support to check
  /// @param var The right hand side of the operator
  /// @return INVALID or BUILTIN
  BinarySupport sint_support(BuiltinID lhs, BinaryOp op, const TypeVariant& var) noexcept;

  /// @brief Check which unary operators are supported by an unsigned int type
  /// @param lhs The left hand side
  /// @param op The unary operator whose support to check
  /// @param var The right hand side of the operator
  /// @return INVALID or BUILTIN
  BinarySupport uint_support(BuiltinID lhs, BinaryOp op, const TypeVariant& var) noexcept;

  /// @brief Check which unary operators are supported by a floating point type
  /// @param lhs The left hand side
  /// @param op The unary operator whose support to check
  /// @param var The right hand side of the operator
  /// @return INVALID or BUILTIN
  BinarySupport fp_support(BuiltinID lhs, BinaryOp op, const TypeVariant& var) noexcept;

  /// @brief Check which unary operators are supported by byte type
  /// @param lhs The left hand side
  /// @param op The unary operator whose support to check
  /// @param var The right hand side of the operator
  /// @return INVALID or BUILTIN
  BinarySupport bytes_support(BuiltinID lhs, BinaryOp op, const TypeVariant& var) noexcept;

  /// @brief Check if the type with ID 'ID' supports 'op'
  /// @param ID The type ID
  /// @param op The operator whose support to check
  /// @param var The right hand side of the operator
  /// @return UnarySupport
  BinarySupport builtin_support(BuiltinID ID, BinaryOp op, const TypeVariant& var) noexcept;

  /************* CONVERSIONS *************/

  /// @brief Check if an error type is castable to another type
  /// @param var The type to cast to
  /// @return BUILTIN
  ConversionSupport error_castable([[maybe_unused]] const TypeVariant& var) noexcept;

  /// @brief Always return INVALID
  /// @param var The type to cast to (unused)
  /// @return INVALID
  ConversionSupport not_castable([[maybe_unused]] const TypeVariant& var) noexcept;

  /// @brief Check if a type is castable to another type
  /// @param var The type to cast to
  /// @return BUILTIN or INVALID
  ConversionSupport builtin_castable(const TypeVariant& var) noexcept;
}

#endif // !HG_COLT_SUPPORT_OP