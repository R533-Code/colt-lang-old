/*****************************************************************//**
 * @file   colt_builtin_id.h
 * @brief  Contains BuiltinID enum that represent the possible
 *         built-in types.
 * 
 * @author RPC
 * @date   February 2024
 *********************************************************************/
#ifndef HG_COLTC_BUILTIN_ID
#define HG_COLTC_BUILTIN_ID

#include "meta/meta_enum.h"

DECLARE_ENUM_WITH_TYPE(u8, clt::lng, BuiltinID,
  BOOL,
  CHAR,
  U8, U16, U32, U64,
  I8, I16, I32, I64,
  F32, F64,
  BYTE, WORD, DWORD, QWORD
);

namespace clt::lng
{
  /// @brief Check if an ID represents a bool
  /// @param id The ID to check for
  /// @return True if BOOL
  constexpr bool isBool(BuiltinID id) noexcept
  {
    return id == BuiltinID::BOOL;
  }

  /// @brief Check if an ID represents a char
  /// @param id The ID to check for
  /// @return True if CHAR
  constexpr bool isChar(BuiltinID id) noexcept
  {
    return id == BuiltinID::CHAR;
  }

  /// @brief Check if an ID represents a unsigned integer
  /// @param id The ID to check for 
  /// @return True if u(8|16|32|64)
  constexpr bool isUInt(BuiltinID id) noexcept
  {
    return BuiltinID::U8 <= id && id <= BuiltinID::U64;
  }

  /// @brief Check if an ID represents a signed integer
  /// @param id The ID to check for 
  /// @return True if i(8|16|32|64)
  constexpr bool isSInt(BuiltinID id) noexcept
  {
    return BuiltinID::I8 <= id && id <= BuiltinID::I64;
  }

  /// @brief Check if an ID represents an integer (signed or unsigned)
  /// @param id The ID to check for
  /// @return True if [iu](8|16|32|64)
  constexpr bool isIntegral(BuiltinID id) noexcept
  {
    return BuiltinID::U8 <= id && id <= BuiltinID::I64;
  }

  /// @brief Check if an ID represents a byte type
  /// @param id The ID to check for
  /// @return True if BYTE or [DQ]?WORD
  constexpr bool isBytes(BuiltinID id) noexcept
  {
    return BuiltinID::BYTE <= id && id <= BuiltinID::QWORD;
  }
  
  /// @brief Check if an ID represents a floating point type
  /// @param id The ID to check for
  /// @return True if F32 or F64
  constexpr bool isFP(BuiltinID id) noexcept
  {
    return BuiltinID::F32 == id || id == BuiltinID::F64;
  }

  /// @brief The type of function checks on BuiltinID
  using BuilinTypeCheck_t = bool (*)(BuiltinID) noexcept;
}

#endif // !HG_COLTC_BUILTIN_ID
