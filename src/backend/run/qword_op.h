/*****************************************************************/ /**
 * @file   qword_op.h
 * @brief  Contains functions that applies operations to QWORD_t.
 * The functions provided allows to apply checked operations
 * on QWORD_t. This is useful for interpreters and constant folding.
 *
 * @author RPC
 * @date   April 2024
 *********************************************************************/
#ifndef HG_COLT_QWORD_OP
#define HG_COLT_QWORD_OP

#include <utility>
#include "common/macros.h"
#include "common/types.h"
#include "meta/meta_enum.h"

#define COLT_TypeOp_PACK                                                   \
  TypeOp::i8_t, TypeOp::i16_t, TypeOp::i32_t, TypeOp::i64_t, TypeOp::u8_t, \
      TypeOp::u16_t, TypeOp::u32_t, TypeOp::u64_t, TypeOp::f32_t, TypeOp::f64_t
#define COLT_Size_PACK Size::_8bits, Size::_16bits, Size::_32bits, Size::_64bits

#define COLT_GENERATE_TABLE_FOR(FnPtr)                 \
  template<typename T, T... Op>                        \
  consteval auto COLT_CONCAT(generate_table_, FnPtr)() \
  {                                                    \
    return std::array{&(FnPtr<Op>)...};                \
  }

#define COLT_TypeOpTable(Name)    \
  clt::run::details::COLT_CONCAT( \
      generate_table_, Name)<clt::run::TypeOp, COLT_TypeOp_PACK>()

#define COLT_TypeOpFnBinary(Name, fnname)                               \
  inline ResultQWORD fnname(QWORD_t a, QWORD_t b, TypeOp type) noexcept \
  {                                                                     \
    static constexpr std::array table = COLT_TypeOpTable(Name);         \
    return table[static_cast<u8>(type)](a, b);                          \
  }

#define COLT_TypeOpFnUnary(Name, fnname)                        \
  inline ResultQWORD fnname(QWORD_t a, TypeOp type) noexcept    \
  {                                                             \
    static constexpr std::array table = COLT_TypeOpTable(Name); \
    return table[static_cast<u8>(type)](a);                     \
  }

/// @brief Helpers for interpreting code
namespace clt::run
{
  enum class TypeOp : u8
  {
    i8_t,
    i16_t,
    i32_t,
    i64_t,
    u8_t,
    u16_t,
    u32_t,
    u64_t,
    f32_t,
    f64_t
  };

  /// @brief Represents the outcome of an operation
  enum OpError : u8
  {
    /// @brief No error
    NO_ERROR,
    /// @brief Invalid operand
    INVALID_OP,
    /// @brief Division or modulo by 0
    DIV_BY_ZERO,
    /// @brief Shift by a size greater than bits size
    SHIFT_BY_GRE_SIZEOF,
    /// @brief Unsigned overflow
    UNSIGNED_OVERFLOW,
    /// @brief Unsigned underflow
    UNSIGNED_UNDERFLOW,
    /// @brief Signed overflow
    SIGNED_OVERFLOW,
    /// @brief Signed underflow
    SIGNED_UNDERFLOW,
    /// @brief The floating point was NaN
    WAS_NAN,
    /// @brief The returned value is NaN
    RET_NAN,
  };

  namespace details
  {
    /// @brief The result of an integer operation
    enum IntOpResult
    {
      OP_VALID,
      OP_OVERFLOW,
      OP_UNDERFLOW
    };

    template<typename T>
      requires std::is_integral_v<T>
    IntOpResult add_int(T a, T x, T& result) noexcept
    {
      if constexpr (std::is_signed_v<T>)
      {
        if (x > 0 && a > std::numeric_limits<T>::max() - x)
          return OP_OVERFLOW;
        if (x < 0 && a < std::numeric_limits<T>::min() - x)
          return OP_UNDERFLOW;
        result = a + x;
        return OP_VALID;
      }
      else
      {
        if constexpr (!std::is_same_v<T, uint64_t>)
        {
          uint64_t res = a;
          res += x;
          result = (T)res;
          if (res & (std::numeric_limits<uint64_t>::max() + 1))
            return OP_OVERFLOW;
          return OP_VALID;
        }
        else
        {
          uint64_t res = a;
          res += x;
          result = res;
          if (std::max(a, x) > res)
            return OP_OVERFLOW;
          return OP_VALID;
        }
      }
    }

    template<typename T>
      requires std::is_integral_v<T>
    IntOpResult sub_int(T a, T x, T& result) noexcept
    {
      if constexpr (std::is_signed_v<T>)
      {
        if (x < 0 && a > std::numeric_limits<T>::max() + x)
          return OP_OVERFLOW;
        if (x > 0 && a < std::numeric_limits<T>::min() + x)
          return OP_UNDERFLOW;
        result = a - x;
        return OP_VALID;
      }
      else
      {
        result = a - x;
        if (x > a)
          return OP_UNDERFLOW;
        return OP_VALID;
      }
    }

    template<typename T>
      requires std::is_integral_v<T>
    IntOpResult mul_int(T a, T x, T& result) noexcept
    {
      if constexpr (std::is_signed_v<T>)
      {
        if (a == -1 && x == std::numeric_limits<T>::min())
          return OP_OVERFLOW;
        if (x == -1 && a == std::numeric_limits<T>::min())
          return OP_OVERFLOW;
        if (x > 0
            && (a > std::numeric_limits<T>::max() / x
                || a < std::numeric_limits<T>::min() / x))
          return OP_OVERFLOW;
        if (x < 0
            && (a < std::numeric_limits<T>::max() / x
                || a > std::numeric_limits<T>::min() / x))
          return OP_UNDERFLOW;
        result = a * x;
        return OP_VALID;
      }
      else
      {
        if constexpr (!std::is_same_v<T, uint64_t>)
        {
          uint64_t res = a;
          res += x;
          result = static_cast<T>(res);
          if (res
              & (std::numeric_limits<uint64_t>::max()
                 & ~static_cast<uint64_t>(std::numeric_limits<T>::max())))
            return OP_OVERFLOW;
          return OP_VALID;
        }
        else
        {
          uint64_t res = a;
          res *= x;
          result = res;
          if (a != 0 && res / a != x)
            return OP_OVERFLOW;
          return OP_VALID;
        }
      }
    }

    template<typename T>
      requires std::is_integral_v<T>
    IntOpResult div_int(T a, T x, T& result) noexcept
    {
      if constexpr (std::is_signed_v<T>)
      {
        if (x == -1 && a == std::numeric_limits<T>::min())
          return OP_OVERFLOW;
        result = a / x;
        return OP_VALID;
      }
      else
      {
        result = a / x;
        return OP_VALID;
      }
    }

    template<typename T>
      requires std::is_integral_v<T>
    IntOpResult mod_int(T a, T x, T& result) noexcept
    {
      if constexpr (std::is_signed_v<T>)
      {
        if (x == -1 && a == std::numeric_limits<T>::min())
          return OP_OVERFLOW;
        result = a % x;
        return OP_VALID;
      }
      else
      {
        result = a % x;
        return OP_VALID;
      }
    }

    template<typename T>
    constexpr OpError int_op_to_op_error(IntOpResult res) noexcept
    {
      switch_no_default(res)
      {
      case OP_VALID:
        return NO_ERROR;
      case OP_OVERFLOW:
        return std::is_signed_v<T> ? SIGNED_OVERFLOW : UNSIGNED_OVERFLOW;
      case OP_UNDERFLOW:
        return std::is_signed_v<T> ? SIGNED_UNDERFLOW : UNSIGNED_UNDERFLOW;
      }
    }
  } // namespace details

  constexpr u8 to_sizeof(TypeOp op) noexcept
  {
    using enum clt::run::TypeOp;

    switch_no_default(op)
    {
    case i8_t:
    case u8_t:
      return 8;
    case i16_t:
    case u16_t:
      return 16;
    case i32_t:
    case u32_t:
    case f32_t:
      return 32;
    case i64_t:
    case u64_t:
    case f64_t:
      return 64;
    }
  }

  template<TypeOp Type>
  struct TypeOp_to_type
  {
  };
  template<>
  struct TypeOp_to_type<TypeOp::u8_t>
  {
    using type = u8;
  };
  template<>
  struct TypeOp_to_type<TypeOp::u16_t>
  {
    using type = u16;
  };
  template<>
  struct TypeOp_to_type<TypeOp::u32_t>
  {
    using type = u32;
  };
  template<>
  struct TypeOp_to_type<TypeOp::u64_t>
  {
    using type = u64;
  };
  template<>
  struct TypeOp_to_type<TypeOp::i8_t>
  {
    using type = i8;
  };
  template<>
  struct TypeOp_to_type<TypeOp::i16_t>
  {
    using type = i16;
  };
  template<>
  struct TypeOp_to_type<TypeOp::i32_t>
  {
    using type = i32;
  };
  template<>
  struct TypeOp_to_type<TypeOp::i64_t>
  {
    using type = i64;
  };
  template<>
  struct TypeOp_to_type<TypeOp::f32_t>
  {
    using type = f32;
  };
  template<>
  struct TypeOp_to_type<TypeOp::f64_t>
  {
    using type = f64;
  };
  template<TypeOp Type>
  using TypeOp_to_type_t = typename TypeOp_to_type<Type>::type;

  /// @brief Converts an 'OpError' to a string explaining it
  /// @param err The OpError to explain
  /// @return String explaining the error
  constexpr const char* toExplanation(OpError err) noexcept
  {
    switch_no_default(err)
    {
    case NO_ERROR:
      return "No errors detected!";
    case INVALID_OP:
      return "Invalid operand type for operation!";
    case DIV_BY_ZERO:
      return "Integral division by zero!";
    case SHIFT_BY_GRE_SIZEOF:
      return "Shift by value greater than bits size!";
    case UNSIGNED_OVERFLOW:
      return "Unsigned overflow detected!";
    case UNSIGNED_UNDERFLOW:
      return "Unsigned underflow detected!";
    case SIGNED_OVERFLOW:
      return "Signed overflow detected!";
    case SIGNED_UNDERFLOW:
      return "Signed underflow detected!";
    case WAS_NAN:
      return "Floating point value was NaN!";
    case RET_NAN:
      return "Floating point operation evaluates to NaN!";
    }
  }

  /// @brief The result of any operation
  using ResultQWORD = std::pair<QWORD_t, OpError>;

  /***************** BINARY *****************/

  template<TypeOp Op>
  ResultQWORD templated_add(QWORD_t a, QWORD_t b) noexcept
  {
    using Ty = TypeOp_to_type_t<Op>;
    if constexpr (std::is_floating_point_v<Ty>)
    {
      if (std::isnan(a.as<Ty>()))
        return {a, WAS_NAN};
      if (std::isnan(b.as<Ty>()))
        return {b, WAS_NAN};
      a.bit_assign(a.as<Ty>() + b.as<Ty>());
      return {a, std::isnan(a.as<Ty>()) ? RET_NAN : NO_ERROR};
    }
    else
    {
      Ty ret     = 0;
      auto error = details::int_op_to_op_error<Ty>(
          details::add_int(a.as<Ty>(), b.as<Ty>(), ret));
      a.bit_assign(ret);
      return {a, error};
    }
  }

  template<TypeOp Op>
  ResultQWORD templated_sub(QWORD_t a, QWORD_t b) noexcept
  {
    using Ty = TypeOp_to_type_t<Op>;
    if constexpr (std::is_floating_point_v<Ty>)
    {
      if (std::isnan(a.as<Ty>()))
        return {a, WAS_NAN};
      if (std::isnan(b.as<Ty>()))
        return {b, WAS_NAN};
      a.bit_assign(a.as<Ty>() - b.as<Ty>());
      return {a, std::isnan(a.as<Ty>()) ? RET_NAN : NO_ERROR};
    }
    else
    {
      Ty ret     = 0;
      auto error = details::int_op_to_op_error<Ty>(
          details::sub_int(a.as<Ty>(), b.as<Ty>(), ret));
      a.bit_assign(ret);
      return {a, error};
    }
  }

  template<TypeOp Op>
  ResultQWORD templated_mul(QWORD_t a, QWORD_t b) noexcept
  {
    using Ty = TypeOp_to_type_t<Op>;
    if constexpr (std::is_floating_point_v<Ty>)
    {
      if (std::isnan(a.as<Ty>()))
        return {a, WAS_NAN};
      if (std::isnan(b.as<Ty>()))
        return {b, WAS_NAN};
      a.bit_assign(a.as<Ty>() * b.as<Ty>());
      return {a, std::isnan(a.as<Ty>()) ? RET_NAN : NO_ERROR};
    }
    else
    {
      Ty ret     = 0;
      auto error = details::int_op_to_op_error<Ty>(
          details::mul_int(a.as<Ty>(), b.as<Ty>(), ret));
      a.bit_assign(ret);
      return {a, error};
    }
  }

  template<TypeOp Op>
  ResultQWORD templated_div(QWORD_t a, QWORD_t b) noexcept
  {
    using Ty = TypeOp_to_type_t<Op>;
    if constexpr (std::is_floating_point_v<Ty>)
    {
      if (std::isnan(a.as<Ty>()))
        return {a, WAS_NAN};
      if (std::isnan(b.as<Ty>()))
        return {b, WAS_NAN};
      a.bit_assign(a.as<Ty>() / b.as<Ty>());
      return {a, std::isnan(a.as<Ty>()) ? RET_NAN : NO_ERROR};
    }
    else
    {
      if (b.as<Ty>() == 0)
        return {a, DIV_BY_ZERO};
      Ty ret     = 0;
      auto error = details::int_op_to_op_error<Ty>(
          details::div_int(a.as<Ty>(), b.as<Ty>(), ret));
      a.bit_assign(ret);
      return {a, error};
    }
  }

  template<TypeOp Op>
  ResultQWORD templated_mod(QWORD_t a, QWORD_t b) noexcept
  {
    using Ty = TypeOp_to_type_t<Op>;
    if constexpr (std::is_floating_point_v<Ty>)
      return {a, INVALID_OP};
    else
    {
      if (b.as<Ty>() == 0)
        return {a, DIV_BY_ZERO};
      Ty ret        = 0;
      OpError error = details::int_op_to_op_error<Ty>(
          details::mod_int(a.as<Ty>(), b.as<Ty>(), ret));
      a.bit_assign(ret);
      return {a, error};
    }
  }

  template<TypeOp Op>
  ResultQWORD templated_neg(QWORD_t a) noexcept
  {
    using Ty = TypeOp_to_type_t<Op>;
    if constexpr (std::is_floating_point_v<Ty>)
    {
      if (std::isnan(a.as<Ty>()))
        return {a, WAS_NAN};
      a.bit_assign(-a.as<Ty>());
      return {a, std::isnan(a.as<Ty>()) ? RET_NAN : NO_ERROR};
    }
    else if constexpr (std::is_signed_v<Ty>)
    {
      if (a.as<Ty>() == std::numeric_limits<Ty>::min())
        return {a.as<Ty>(), SIGNED_UNDERFLOW};
      a.bit_assign(-a.as<Ty>());
      return {a, NO_ERROR};
    }
    else
      return {a, INVALID_OP};
  }

  /*************** COMPARISON ***************/

  template<TypeOp Op>
  ResultQWORD templated_eq(QWORD_t a, QWORD_t b) noexcept
  {
    using Ty = TypeOp_to_type_t<Op>;
    QWORD_t ret;
    ret.bit_assign(a.as<Ty>() == b.as<Ty>());
    if constexpr (std::is_floating_point_v<Ty>)
    {
      if (std::isnan(a.as<Ty>()) || std::isnan(b.as<Ty>()))
        return {ret, WAS_NAN};
    }
    return {ret, NO_ERROR};
  }

  template<TypeOp Op>
  ResultQWORD templated_neq(QWORD_t a, QWORD_t b) noexcept
  {
    using Ty = TypeOp_to_type_t<Op>;
    QWORD_t ret;
    ret.bit_assign(a.as<Ty>() != b.as<Ty>());
    if constexpr (std::is_floating_point_v<Ty>)
    {
      if (std::isnan(a.as<Ty>()) || std::isnan(b.as<Ty>()))
        return {ret, WAS_NAN};
    }
    return {ret, NO_ERROR};
  }

  template<TypeOp Op>
  ResultQWORD templated_le(QWORD_t a, QWORD_t b) noexcept
  {
    using Ty = TypeOp_to_type_t<Op>;
    QWORD_t ret;
    ret.bit_assign(a.as<Ty>() < b.as<Ty>());
    if constexpr (std::is_floating_point_v<Ty>)
    {
      if (std::isnan(a.as<Ty>()) || std::isnan(b.as<Ty>()))
        return {ret, WAS_NAN};
    }
    return {ret, NO_ERROR};
  }

  template<TypeOp Op>
  ResultQWORD templated_ge(QWORD_t a, QWORD_t b) noexcept
  {
    using Ty = TypeOp_to_type_t<Op>;
    QWORD_t ret;
    ret.bit_assign(a.as<Ty>() > b.as<Ty>());
    if constexpr (std::is_floating_point_v<Ty>)
    {
      if (std::isnan(a.as<Ty>()) || std::isnan(b.as<Ty>()))
        return {ret, WAS_NAN};
    }
    return {ret, NO_ERROR};
  }

  template<TypeOp Op>
  ResultQWORD templated_leq(QWORD_t a, QWORD_t b) noexcept
  {
    using Ty = TypeOp_to_type_t<Op>;
    QWORD_t ret;
    ret.bit_assign(a.as<Ty>() <= b.as<Ty>());
    if constexpr (std::is_floating_point_v<Ty>)
    {
      if (std::isnan(a.as<Ty>()) || std::isnan(b.as<Ty>()))
        return {ret, WAS_NAN};
    }
    return {ret, NO_ERROR};
  }

  template<TypeOp Op>
  ResultQWORD templated_geq(QWORD_t a, QWORD_t b) noexcept
  {
    using Ty = TypeOp_to_type_t<Op>;
    QWORD_t ret;
    ret.bit_assign(a.as<Ty>() >= b.as<Ty>());
    if constexpr (std::is_floating_point_v<Ty>)
    {
      if (std::isnan(a.as<Ty>()) || std::isnan(b.as<Ty>()))
        return {ret, WAS_NAN};
    }
    return {ret, NO_ERROR};
  }

  /***************** BITWISE *****************/

  namespace details
  {
    constexpr u64 generate_n_bits_1(u8 bits) noexcept
    {
      return ((u64)2 << (u64)bits) - 1;
    }
  } // namespace details

  inline ResultQWORD bit_and(QWORD_t a, QWORD_t b, u8 bits) noexcept
  {
    return {(a & b) & details::generate_n_bits_1(bits), NO_ERROR};
  }

  inline ResultQWORD bit_or(QWORD_t a, QWORD_t b, u8 bits) noexcept
  {
    return {(a | b) & details::generate_n_bits_1(bits), NO_ERROR};
  }

  inline ResultQWORD bit_xor(QWORD_t a, QWORD_t b, u8 bits) noexcept
  {
    return {(a ^ b) & details::generate_n_bits_1(bits), NO_ERROR};
  }

  inline ResultQWORD lsr(QWORD_t a, QWORD_t b, u8 bits) noexcept
  {
    QWORD_t result;
    result.bit_assign(
        (a.as<u64>() >> b.as<u64>()) & details::generate_n_bits_1(bits));
    return {result, b.as<u64>() < bits ? NO_ERROR : SHIFT_BY_GRE_SIZEOF};
  }

  inline ResultQWORD lsl(QWORD_t a, QWORD_t b, u8 bits) noexcept
  {
    QWORD_t result;
    result.bit_assign(
        (a.as<u64>() << b.as<u64>()) & details::generate_n_bits_1(bits));
    return {result, b.as<u64>() < bits ? NO_ERROR : SHIFT_BY_GRE_SIZEOF};
  }

  inline ResultQWORD asr(QWORD_t a, QWORD_t b, u8 bits) noexcept
  {
    QWORD_t result    = 0;
    const u64 bitmask = details::generate_n_bits_1(bits);
    a &= bitmask;
    if (a.is_set(bits))
    {
      result = ~result;
      result &= a.as<u64>() >> (b.as<u64>() | (bitmask << b.as<u64>()));
    }
    else
      result.bit_assign(a.as<u64>() >> b.as<u64>());
    return {result & bitmask, b.as<u64>() < bits ? NO_ERROR : SHIFT_BY_GRE_SIZEOF};
  }

  inline ResultQWORD bit_not(QWORD_t a, u8 bits) noexcept
  {
    a = ~a;
    // We turn off the bits that shouldn't be on
    a &= details::generate_n_bits_1(bits);
    return {a, NO_ERROR};
  }

  /***************** CONVERSIONS *****************/

  template<TypeOp From, TypeOp To>
  ResultQWORD templated_cnv(QWORD_t a) noexcept
  {
    using From_t = TypeOp_to_type_t<From>;
    using To_t   = TypeOp_to_type_t<To>;
    if constexpr (std::is_floating_point_v<From_t>)
      if (std::isnan(a.as<From_t>()))
        return {{}, WAS_NAN};
    if constexpr (
        std::is_floating_point_v<From_t> && !std::is_floating_point_v<To_t>)
    {
      if constexpr (std::is_unsigned_v<To_t>)
      {
        constexpr auto MAX_VALUE =
            static_cast<From_t>(std::numeric_limits<To_t>::max() / 2 + 1)
            * static_cast<From_t>(2.0);
        if (a.as<From_t>() < static_cast<From_t>(0.0))
          return {{}, UNSIGNED_UNDERFLOW};
        if (!(a.as<From_t>() - MAX_VALUE < static_cast<From_t>(-0.5)))
        {
          a.bit_assign(std::numeric_limits<To_t>::max());
          return {a, UNSIGNED_OVERFLOW};
        }
      }
      else if constexpr (std::is_signed_v<To_t>)
      {
        constexpr auto MAX_VALUE =
            static_cast<From_t>(std::numeric_limits<To_t>::max() / 2 + 1)
            * static_cast<From_t>(2.0);
        if (!(a.as<From_t>() - MAX_VALUE < static_cast<From_t>(-0.5)))
        {
          a.bit_assign(std::numeric_limits<To_t>::max());
          return {a, SIGNED_OVERFLOW};
        }
        if (!(a.as<From_t>() - static_cast<From_t>(std::numeric_limits<To_t>::min())
              > static_cast<From_t>(-0.5)))
        {
          a.bit_assign(std::numeric_limits<To_t>::min());
          return {std::numeric_limits<To_t>::min(), SIGNED_UNDERFLOW};
        }
      }
    }
    a.bit_assign(static_cast<To_t>(a.as<From_t>()));
    return {a, NO_ERROR};
  }

  /***************** UTILITIES *****************/

  /// @brief QWORD_t binary instruction function type
  using BinaryInst_t = ResultQWORD (*)(QWORD_t, QWORD_t, TypeOp) noexcept;

  /// @brief Helper to avoid casting nullptr to BinaryInst_t type
  inline constexpr BinaryInst_t nullbinary = nullptr;

  /// @brief QWORD_t unary instruction function type
  using UnaryInst_t = ResultQWORD (*)(QWORD_t, TypeOp) noexcept;

  /// @brief Helper to avoid casting nullptr to UnaryInst_t type
  inline constexpr UnaryInst_t nullunary = nullptr;

  /// @brief Check if a TypeOp is a signed integer
  /// @param op The type
  /// @return True if signed integer
  constexpr bool is_sint(TypeOp op) noexcept
  {
    return TypeOp::i8_t <= op && op <= TypeOp::i64_t;
  }

  /// @brief Check if a TypeOp is an unsigned integer
  /// @param op The type
  /// @return True if unsigned integer
  constexpr bool is_uint(TypeOp op) noexcept
  {
    return TypeOp::u8_t <= op && op <= TypeOp::u64_t;
  }

  /// @brief Check if a TypeOp is an integer
  /// @param op The type
  /// @return True if signed or unsigned integer
  constexpr bool isInt(TypeOp op) noexcept
  {
    return TypeOp::i8_t <= op && op <= TypeOp::u64_t;
  }

  /// @brief Check if a TypeOp is a floating point
  /// @param op The type
  /// @return True if TypeOp is f32_t or f64_t
  constexpr bool is_fp(TypeOp op) noexcept
  {
    return TypeOp::f32_t == op || TypeOp::f64_t == op;
  }

  namespace details
  {
    /***********************************************
    * The code below generates tables that converts
    * the templated functions to non-templated
    * functions with a runtime parameter choosing
    * the right templated function.
    ***********************************************/

    COLT_GENERATE_TABLE_FOR(templated_add);
    COLT_GENERATE_TABLE_FOR(templated_sub);
    COLT_GENERATE_TABLE_FOR(templated_mul);
    COLT_GENERATE_TABLE_FOR(templated_div);
    COLT_GENERATE_TABLE_FOR(templated_mod);
    COLT_GENERATE_TABLE_FOR(templated_neg);

    COLT_GENERATE_TABLE_FOR(templated_eq);
    COLT_GENERATE_TABLE_FOR(templated_neq);
    COLT_GENERATE_TABLE_FOR(templated_le);
    COLT_GENERATE_TABLE_FOR(templated_ge);
    COLT_GENERATE_TABLE_FOR(templated_leq);
    COLT_GENERATE_TABLE_FOR(templated_geq);

    template<TypeOp Op, TypeOp... Ty>
    consteval auto generate_cnv_row()
    {
      return std::array{&templated_cnv<Op, Ty>...};
    }

    template<TypeOp... Ty>
    consteval auto generate_cnv()
    {
      // We need to generate a matrix of all possible conversions
      return std::array{generate_cnv_row<Ty, Ty...>()...};
    }
  } // namespace details

  /// @brief Adds two QWORDs
  /// @param a The first operand
  /// @param b The second operand
  /// @param type The type of both QWORDs
  /// @return The result of the operation
  COLT_TypeOpFnBinary(templated_add, add);
  /// @brief Subtracts two QWORDs
  /// @param a The first operand
  /// @param b The second operand
  /// @param type The type of both QWORDs
  /// @return The result of the operation
  COLT_TypeOpFnBinary(templated_sub, sub);
  /// @brief Multiplies two QWORDs
  /// @param a The first operand
  /// @param b The second operand
  /// @param type The type of both QWORDs
  /// @return The result of the operation
  COLT_TypeOpFnBinary(templated_mul, mul);
  /// @brief Divides two QWORDs
  /// @param a The first operand
  /// @param b The second operand
  /// @param type The type of both QWORDs
  /// @return The result of the operation
  COLT_TypeOpFnBinary(templated_div, div);
  /// @brief Returns the remainder of the division of two QWORDs
  /// @param a The first operand
  /// @param b The second operand
  /// @param type The type of both QWORDs
  /// @return The result of the operation
  COLT_TypeOpFnBinary(templated_mod, mod);
  /// @brief Compares two QWORDs for equality
  /// @param a The first operand
  /// @param b The second operand
  /// @param type The type of both QWORDs
  /// @return The result of the operation
  COLT_TypeOpFnBinary(templated_eq, eq);
  /// @brief Compares two QWORDs for inequality
  /// @param a The first operand
  /// @param b The second operand
  /// @param type The type of both QWORDs
  /// @return The result of the operation
  COLT_TypeOpFnBinary(templated_neq, neq);
  /// @brief Compares two QWORDs for less-than
  /// @param a The first operand
  /// @param b The second operand
  /// @param type The type of both QWORDs
  /// @return The result of the operation
  COLT_TypeOpFnBinary(templated_le, le);
  /// @brief Compares two QWORDs for greater-than
  /// @param a The first operand
  /// @param b The second operand
  /// @param type The type of both QWORDs
  /// @return The result of the operation
  COLT_TypeOpFnBinary(templated_ge, ge);
  /// @brief Compares two QWORDs for less-than-equal
  /// @param a The first operand
  /// @param b The second operand
  /// @param type The type of both QWORDs
  /// @return The result of the operation
  COLT_TypeOpFnBinary(templated_leq, leq);
  /// @brief Compares two QWORDs for greater-than-equal
  /// @param a The first operand
  /// @param b The second operand
  /// @param type The type of both QWORDs
  /// @return The result of the operation
  COLT_TypeOpFnBinary(templated_geq, geq);
  /// @brief Negates a QWORDs
  /// @param a The first operand
  /// @param type The type of the QWORDs
  /// @return The result of the operation
  COLT_TypeOpFnUnary(templated_neg, neg);

  /// @brief Converts a QWORD from a type to another
  /// @param value The value to convert
  /// @param from The type to convert from
  /// @param to The type to convert to
  /// @return The conversion result
  inline ResultQWORD cnv(QWORD_t value, TypeOp from, TypeOp to) noexcept
  {
    static constexpr auto cnv = details::generate_cnv<COLT_TypeOp_PACK>();
    return cnv[(u8)from][(u8)to](value);
  }
} // namespace clt::run

#endif //!HG_COLT_QWORD_OP