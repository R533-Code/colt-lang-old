/*****************************************************************/ /**
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
#include "common/assertions.h"

DECLARE_ENUM_WITH_TYPE(
    u8, clt::lng, BuiltinID, BOOL, CHAR, U8, U16, U32, U64, I8, I16, I32, I64, F32,
    F64, BYTE, WORD, DWORD, QWORD);

namespace clt::lng
{
  /// @brief Check if an ID represents a bool
  /// @param id The ID to check for
  /// @return True if BOOL
  constexpr bool is_bool(BuiltinID id) noexcept
  {
    return id == BuiltinID::BOOL;
  }

  /// @brief Check if an ID represents a char
  /// @param id The ID to check for
  /// @return True if CHAR
  constexpr bool is_char(BuiltinID id) noexcept
  {
    return id == BuiltinID::CHAR;
  }

  /// @brief Check if an ID represents a unsigned integer
  /// @param id The ID to check for
  /// @return True if u(8|16|32|64)
  constexpr bool is_uint(BuiltinID id) noexcept
  {
    return BuiltinID::U8 <= id && id <= BuiltinID::U64;
  }

  /// @brief Check if an ID represents a signed integer
  /// @param id The ID to check for
  /// @return True if i(8|16|32|64)
  constexpr bool is_sint(BuiltinID id) noexcept
  {
    return BuiltinID::I8 <= id && id <= BuiltinID::I64;
  }

  /// @brief Check if an ID represents an integer (signed or unsigned)
  /// @param id The ID to check for
  /// @return True if [iu](8|16|32|64)
  constexpr bool is_integral(BuiltinID id) noexcept
  {
    return BuiltinID::U8 <= id && id <= BuiltinID::I64;
  }

  /// @brief Check if an ID represents a byte type
  /// @param id The ID to check for
  /// @return True if BYTE or [DQ]?WORD
  constexpr bool is_bytes(BuiltinID id) noexcept
  {
    return BuiltinID::BYTE <= id && id <= BuiltinID::QWORD;
  }

  /// @brief Check if an ID represents a floating point type
  /// @param id The ID to check for
  /// @return True if F32 or F64
  constexpr bool is_fp(BuiltinID id) noexcept
  {
    return BuiltinID::F32 == id || id == BuiltinID::F64;
  }

  /// @brief The type of function checks on BuiltinID
  using BuilinTypeCheck_t = bool (*)(BuiltinID) noexcept;

  /// @brief Struct with custom formatting
  struct TypedQWORD
  {
    /// @brief The value of the literal
    QWORD_t value;
    /// @brief The type of the QWORD_t
    BuiltinID ID;
  };
} // namespace clt::lng

template<>
struct fmt::formatter<clt::lng::TypedQWORD> : clt::meta::DefaultParserFMT
{
  template<typename context>
  auto format(const clt::lng::TypedQWORD& c, context& ctx) const
  {
    using enum clt::lng::BuiltinID;

    switch_no_default(c.ID)
    {
    case BOOL:
      return fmt::format_to(ctx.out(), "{}", c.value.as<bool>());
    case CHAR:
      return fmt::format_to(ctx.out(), "{}", c.value.as<char>());
    case U8:
    case BYTE:
      return fmt::format_to(ctx.out(), "{}", c.value.as<u8>());
    case U16:
    case WORD:
      return fmt::format_to(ctx.out(), "{}", c.value.as<u16>());
    case U32:
    case DWORD:
      return fmt::format_to(ctx.out(), "{}", c.value.as<u32>());
    case U64:
    case QWORD:
      return fmt::format_to(ctx.out(), "{}", c.value.as<u64>());
    case I8:
      return fmt::format_to(ctx.out(), "{}", c.value.as<i8>());
    case I16:
      return fmt::format_to(ctx.out(), "{}", c.value.as<i16>());
    case I32:
      return fmt::format_to(ctx.out(), "{}", c.value.as<i32>());
    case I64:
      return fmt::format_to(ctx.out(), "{}", c.value.as<i64>());
    case F32:
      return fmt::format_to(ctx.out(), "{}", c.value.as<f32>());
    case F64:
      return fmt::format_to(ctx.out(), "{}", c.value.as<f64>());
    }
  }
};

#endif // !HG_COLTC_BUILTIN_ID
