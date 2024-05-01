/*****************************************************************//**
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
#include "util/macros.h"
#include "meta/meta_enum.h"

#define COLT_TypeOp_PACK TypeOp::i8_t, TypeOp::i16_t, TypeOp::i32_t, TypeOp::i64_t, TypeOp::u8_t, TypeOp::u16_t, TypeOp::u32_t, TypeOp::u64_t, TypeOp::f32_t, TypeOp::f64_t
#define COLT_Size_PACK Size::_8bits, Size::_16bits, Size::_32bits, Size::_64bits

#define COLT_GENERATE_TABLE_FOR(FnPtr) \
template<typename T, T... Op> \
consteval auto COLT_CONCAT(generate_table_, FnPtr)() \
{ \
  return std::array{ &(FnPtr<Op>)... }; \
}

#define COLT_TypeOpTable(Name) clt::run::details::COLT_CONCAT(generate_table_, Name)<clt::run::TypeOp, COLT_TypeOp_PACK>()
#define COLT_SizeTable(Name)   clt::run::details::COLT_CONCAT(generate_table_, Name)<clt::run::Size, COLT_Size_PACK>()

#define COLT_TypeOpFn(Name)  inline ResultQWORD COLT_CONCAT(NT_, Name)(QWORD_t a, QWORD_t b, TypeOp type) noexcept\
{ \
  static constexpr std::array table = COLT_TypeOpTable(Name); \
  return table[static_cast<u8>(type)](a, b); \
}

#define COLT_SizeFn(Name)  inline ResultQWORD COLT_CONCAT(NT_, Name)(QWORD_t a, QWORD_t b, TypeOp type) noexcept\
{ \
  static constexpr std::array table = COLT_SizeTable(Name); \
  return table[static_cast<u8>(TypeOp_to_Size(type))](a, b); \
}

#define COLT_NotTemplateFn(Name) inline  ResultQWORD COLT_CONCAT(NT_, Name)(QWORD_t a, QWORD_t b, TypeOp) noexcept\
{ \
  return Name(a, b); \
}

DECLARE_ENUM_WITH_TYPE(u8, clt::run, TypeOp,
  i8_t, i16_t, i32_t, i64_t,
  u8_t, u16_t, u32_t, u64_t,
  f32_t, f64_t
);

/// @brief Helpers for interpreting code
namespace clt::run
{
  /// @brief Represents the outcome of an operation
  enum OpError
  {
    /// @brief No error
    NO_ERROR,
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
    enum IntOpResult
    {
      OP_VALID,
      OP_OVERFLOW,
      OP_UNDERFLOW
    };

    template<typename T> requires std::is_integral_v<T>
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

    template<typename T> requires std::is_integral_v<T>
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

    template<typename T> requires std::is_integral_v<T>
    IntOpResult mul_int(T a, T x, T& result) noexcept
    {
      if constexpr (std::is_signed_v<T>)
      {
        if (a == -1 && x == std::numeric_limits<T>::min())
          return OP_OVERFLOW;
        if (x == -1 && a == std::numeric_limits<T>::min())
          return OP_OVERFLOW;
        if (x > 0 && (a > std::numeric_limits<T>::max() / x || a < std::numeric_limits<T>::min() / x))
          return OP_OVERFLOW;
        if (x < 0 && (a < std::numeric_limits<T>::max() / x || a > std::numeric_limits<T>::min() / x))
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
          if (res & (std::numeric_limits<uint64_t>::max() & ~static_cast<uint64_t>(std::numeric_limits<T>::max())))
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

    template<typename T> requires std::is_integral_v<T>
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
    constexpr OpError IntOpToOpError(IntOpResult res) noexcept
    {
      switch_no_default (res)
      {
      case OP_VALID:
        return NO_ERROR;
      case OP_OVERFLOW:
        return std::is_signed_v<T> ? SIGNED_OVERFLOW : UNSIGNED_OVERFLOW;
      case OP_UNDERFLOW:
        return std::is_signed_v<T> ? SIGNED_UNDERFLOW : UNSIGNED_UNDERFLOW;
      }
    }
  }

  enum class Size
    : u8
  {
    _8bits, _16bits, _32bits, _64bits
  };

  using enum Size;

  constexpr size_t Size_to_size_t(Size sz) noexcept
  {
    switch_no_default (sz)
    {
    case _8bits:
      return 8;
    case _16bits:
      return 16;
    case _32bits:
      return 32;
    case _64bits:
      return 64;
    }
  }

  constexpr Size TypeOp_to_Size(TypeOp op) noexcept
  {
    using enum clt::run::TypeOp;

    switch_no_default (op)
    {
    case i8_t:
    case u8_t:
      return _8bits;
    case i16_t:
    case u16_t:
      return _16bits;
    case i32_t:
    case u32_t:
    case f32_t:
      return _32bits;
    case i64_t:
    case u64_t:
    case f64_t:
      return _64bits;
    }
  }

  template<Size BitSize> struct size_to_uint {};
  template<> struct size_to_uint<_8bits> { using type = u8; };
  template<> struct size_to_uint<_16bits> { using type = u16; };
  template<> struct size_to_uint<_32bits> { using type = u32; };
  template<> struct size_to_uint<_64bits> { using type = u64; };

  template<Size BitSize>
  struct size_to_int { using type = std::make_signed_t<typename size_to_uint<BitSize>::type>; };

  template<Size BitSize> struct size_to_float {};
  template<> struct size_to_float<_32bits> { using type = f32; };
  template<> struct size_to_float<_64bits> { using type = f64; };

  template<Size BitSize> using size_to_uint_t = typename size_to_uint<BitSize>::type;
  template<Size BitSize> using size_to_int_t = typename size_to_int<BitSize>::type;
  template<Size BitSize> requires (BitSize == _32bits) || (BitSize == _64bits) using size_to_float_t = typename size_to_float<BitSize>::type;

  template<TypeOp Type> struct TypeOp_to_type {};
  template<> struct TypeOp_to_type<TypeOp::u8_t> { using type = u8; };
  template<> struct TypeOp_to_type<TypeOp::u16_t> { using type = u16; };
  template<> struct TypeOp_to_type<TypeOp::u32_t> { using type = u32; };
  template<> struct TypeOp_to_type<TypeOp::u64_t> { using type = u64; };
  template<> struct TypeOp_to_type<TypeOp::i8_t> { using type = i8; };
  template<> struct TypeOp_to_type<TypeOp::i16_t> { using type = i16; };
  template<> struct TypeOp_to_type<TypeOp::i32_t> { using type = i32; };
  template<> struct TypeOp_to_type<TypeOp::i64_t> { using type = i64; };
  template<> struct TypeOp_to_type<TypeOp::f32_t> { using type = f32; };
  template<> struct TypeOp_to_type<TypeOp::f64_t> { using type = f64; };
  template<TypeOp Type> using TypeOp_to_type_t = typename TypeOp_to_type<Type>::type;

  /// @brief Converts an 'OpError' to a string explaining it
  /// @param err The OpError to explain
  /// @return String explaining the error
  constexpr const char* toExplanation(OpError err) noexcept
  {
    switch_no_default (err)
    {
    case NO_ERROR:
      return "No errors detected!";
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
  ResultQWORD add(QWORD_t a, QWORD_t b) noexcept
  {
    using Ty = TypeOp_to_type_t<Op>;
    if constexpr (std::is_floating_point_v<Ty>)
    {
      if (std::isnan(a.as<Ty>()))
        return { a, WAS_NAN };
      if (std::isnan(b.as<Ty>()))
        return { b, WAS_NAN };
      a.bit_assign(a.as<Ty>() + b.as<Ty>());
      return { a, std::isnan(a.as<Ty>()) ? RET_NAN : NO_ERROR };
    }
    else
    {
      Ty ret = 0;
      auto error = details::IntOpToOpError<Ty>(details::add_int(a.as<Ty>(), b.as<Ty>(), ret));
      a.bit_assign(ret);
      return { a, error };
    }
  }


  template<TypeOp Op>
  ResultQWORD sub(QWORD_t a, QWORD_t b) noexcept
  {
    using Ty = TypeOp_to_type_t<Op>;
    if constexpr (std::is_floating_point_v<Ty>)
    {
      if (std::isnan(a.as<Ty>()))
        return { a, WAS_NAN };
      if (std::isnan(b.as<Ty>()))
        return { b, WAS_NAN };
      a.bit_assign(a.as<Ty>() - b.as<Ty>());
      return { a, std::isnan(a.as<Ty>()) ? RET_NAN : NO_ERROR };
    }
    else
    {
      Ty ret = 0;
      auto error = details::IntOpToOpError<Ty>(details::sub_int(a.as<Ty>(), b.as<Ty>(), ret));
      a.bit_assign(ret);
      return { a, error };
    }
  }


  template<TypeOp Op>
  ResultQWORD mul(QWORD_t a, QWORD_t b) noexcept
  {
    using Ty = TypeOp_to_type_t<Op>;
    if constexpr (std::is_floating_point_v<Ty>)
    {
      if (std::isnan(a.as<Ty>()))
        return { a, WAS_NAN };
      if (std::isnan(b.as<Ty>()))
        return { b, WAS_NAN };
      a.bit_assign(a.as<Ty>() * b.as<Ty>());
      return { a, std::isnan(a.as<Ty>()) ? RET_NAN : NO_ERROR };
    }
    else
    {
      Ty ret = 0;
      auto error = details::IntOpToOpError<Ty>(details::mul_int(a.as<Ty>(), b.as<Ty>(), ret));
      a.bit_assign(ret);
      return { a, error };
    }
  }


  template<TypeOp Op>
  ResultQWORD div(QWORD_t a, QWORD_t b) noexcept
  {
    using Ty = TypeOp_to_type_t<Op>;
    if constexpr (std::is_floating_point_v<Ty>)
    {
      if (std::isnan(a.as<Ty>()))
        return { a, WAS_NAN };
      if (std::isnan(b.as<Ty>()))
        return { b, WAS_NAN };
      a.bit_assign(a.as<Ty>() / b.as<Ty>());
      return { a, std::isnan(a.as<Ty>()) ? RET_NAN : NO_ERROR };
    }
    else
    {
      if (b.as<Ty>() == 0)
        return { a, DIV_BY_ZERO };
      Ty ret = 0;
      auto error = details::IntOpToOpError<Ty>(details::div_int(a.as<Ty>(), b.as<Ty>(), ret));
      a.bit_assign(ret);
      return { a, error };
    }
  }


  template<Size BitSize>
  ResultQWORD  mod(QWORD_t a, QWORD_t b) noexcept
  {
    using int_t = size_to_uint_t<BitSize>;
    if (b.as<int_t>() == 0)
      return { a, DIV_BY_ZERO };
    a.bit_assign(a.as<int_t>() % b.as<int_t>());
    return { a, NO_ERROR };
  }


  template<Size BitSize>
  ResultQWORD imod(QWORD_t a, QWORD_t b) noexcept
  {
    using int_t = size_to_int_t<BitSize>;
    if (b.as<int_t>() == 0)
      return { a, DIV_BY_ZERO };
    a.bit_assign(a.as<int_t>() % b.as<int_t>());
    return { a, NO_ERROR };
  }


  inline ResultQWORD bit_and(QWORD_t a, QWORD_t b) noexcept
  {
    return { a & b, NO_ERROR };
  }


  inline ResultQWORD  bit_or(QWORD_t a, QWORD_t b) noexcept
  {
    return { a | b, NO_ERROR };
  }


  inline ResultQWORD bit_xor(QWORD_t a, QWORD_t b) noexcept
  {
    return { a ^ b, NO_ERROR };
  }


  template<Size BitSize>
  ResultQWORD shr(QWORD_t a, QWORD_t b) noexcept
  {
    QWORD_t result;
    result.bit_assign(a.as<u64>() >> b.as<u64>());
    return { result, b.as<u64>() < Size_to_size_t(BitSize) ? NO_ERROR : SHIFT_BY_GRE_SIZEOF };
  }


  template<Size BitSize>
  ResultQWORD shl(QWORD_t a, QWORD_t b) noexcept
  {
    QWORD_t result;
    result.bit_assign(a.as<u64>() << b.as<u64>());
    return { result, b.as<u64>() < Size_to_size_t(BitSize) ? NO_ERROR : SHIFT_BY_GRE_SIZEOF };
  }


  /*************** COMPARISON ***************/

  template<TypeOp Op>
  ResultQWORD eq(QWORD_t a, QWORD_t b) noexcept
  {
    using Ty = TypeOp_to_type_t<Op>;
    QWORD_t ret;
    ret.bit_assign(a.as<Ty>() == b.as<Ty>());
    if constexpr (std::is_floating_point_v<Ty>)
    {
      if (std::isnan(a.as<Ty>()) || std::isnan(b.as<Ty>()))
        return { ret, WAS_NAN };
    }
    return { ret, NO_ERROR };
  }


  template<TypeOp Op>
  ResultQWORD neq(QWORD_t a, QWORD_t b) noexcept
  {
    using Ty = TypeOp_to_type_t<Op>;
    QWORD_t ret;
    ret.bit_assign(a.as<Ty>() != b.as<Ty>());
    if constexpr (std::is_floating_point_v<Ty>)
    {
      if (std::isnan(a.as<Ty>()) || std::isnan(b.as<Ty>()))
        return { ret, WAS_NAN };
    }
    return { ret, NO_ERROR };
  }


  template<TypeOp Op>
  ResultQWORD le(QWORD_t a, QWORD_t b) noexcept
  {
    using Ty = TypeOp_to_type_t<Op>;
    QWORD_t ret;
    ret.bit_assign(a.as<Ty>() < b.as<Ty>());
    if constexpr (std::is_floating_point_v<Ty>)
    {
      if (std::isnan(a.as<Ty>()) || std::isnan(b.as<Ty>()))
        return { ret, WAS_NAN };
    }
    return { ret, NO_ERROR };
  }


  template<TypeOp Op>
  ResultQWORD ge(QWORD_t a, QWORD_t b) noexcept
  {
    using Ty = TypeOp_to_type_t<Op>;
    QWORD_t ret;
    ret.bit_assign(a.as<Ty>() > b.as<Ty>());
    if constexpr (std::is_floating_point_v<Ty>)
    {
      if (std::isnan(a.as<Ty>()) || std::isnan(b.as<Ty>()))
        return { ret, WAS_NAN };
    }
    return { ret, NO_ERROR };
  }


  template<TypeOp Op>
  ResultQWORD leq(QWORD_t a, QWORD_t b) noexcept
  {
    using Ty = TypeOp_to_type_t<Op>;
    QWORD_t ret;
    ret.bit_assign(a.as<Ty>() <= b.as<Ty>());
    if constexpr (std::is_floating_point_v<Ty>)
    {
      if (std::isnan(a.as<Ty>()) || std::isnan(b.as<Ty>()))
        return { ret, WAS_NAN };
    }
    return { ret, NO_ERROR };
  }


  template<TypeOp Op>
  ResultQWORD geq(QWORD_t a, QWORD_t b) noexcept
  {
    using Ty = TypeOp_to_type_t<Op>;
    QWORD_t ret;
    ret.bit_assign(a.as<Ty>() >= b.as<Ty>());
    if constexpr (std::is_floating_point_v<Ty>)
    {
      if (std::isnan(a.as<Ty>()) || std::isnan(b.as<Ty>()))
        return { ret, WAS_NAN };
    }
    return { ret, NO_ERROR };
  }


  /***************** UNARY *****************/

  template<Size BitSize>
  ResultQWORD  neg(QWORD_t a) noexcept
  {
    using int_t = size_to_int_t<BitSize>;
    if (a.as<int_t>() == std::numeric_limits<int_t>::min())
      return { a.as<int_t>(), SIGNED_UNDERFLOW };
    a.bit_assign(-a.as<int_t>());
    return { a, NO_ERROR };
  }

  template<Size BitSize>
  ResultQWORD fneg(QWORD_t a) noexcept
  {
    using f_t = size_to_float_t<BitSize>;
    if (std::isnan(a.as<f_t>()))
      return { a, WAS_NAN };
    a.bit_assign(-a.as<f_t>());
    return { a, std::isnan(a.as<f_t>()) ? RET_NAN : NO_ERROR };
  }

  /// @brief Performs a bitwise not
  /// @param a The QWORD on which to perform the bitwise operation
  /// @return Result with no errors
  template<Size BitSize>
  ResultQWORD bit_not(QWORD_t a) noexcept
  {
    a = ~a;
    // We turn off the bits that shouldn't be on
    a &= std::numeric_limits<size_to_uint_t<BitSize>>::max();
    return { a, NO_ERROR };
  }

  /// @brief Performs a bool not
  /// @param a The QWORD on which to perform the bitwise operation
  /// @return Result with no errors
  inline ResultQWORD bool_not(QWORD_t a) noexcept
  {
    a.bit_assign(!a.as<bool>());
    return { a, NO_ERROR };
  }

  /***************** CONVERSIONS *****************/

  template<TypeOp From, TypeOp To>
  ResultQWORD cnv(QWORD_t a) noexcept
  {
    using From_t = TypeOp_to_type_t<From>;
    using To_t = TypeOp_to_type_t<To>;
    if constexpr (std::is_floating_point_v<From_t>)
      if (std::isnan(a.as<From_t>()))
        return { {}, WAS_NAN };
    if constexpr (std::is_floating_point_v<From_t> && !std::is_floating_point_v<To_t>)
    {
      if constexpr (std::is_unsigned_v<To_t>)
      {
        constexpr auto MAX_VALUE = static_cast<From_t>(std::numeric_limits<To_t>::max() / 2 + 1) * static_cast<From_t>(2.0);
        if (a.as<From_t>() < static_cast<From_t>(0.0))
          return { {}, UNSIGNED_UNDERFLOW };
        if (!(a.as<From_t>() - MAX_VALUE < static_cast<From_t>(-0.5)))
        {
          a.bit_assign(std::numeric_limits<To_t>::max());
          return { a, UNSIGNED_OVERFLOW };
        }
      }
      else if constexpr (std::is_signed_v<To_t>)
      {
        constexpr auto MAX_VALUE = static_cast<From_t>(std::numeric_limits<To_t>::max() / 2 + 1) * static_cast<From_t>(2.0);
        if (!(a.as<From_t>() - MAX_VALUE < static_cast<From_t>(-0.5)))
        {
          a.bit_assign(std::numeric_limits<To_t>::max());
          return { a, SIGNED_OVERFLOW };
        }
        if (!(a.as<From_t>() - static_cast<From_t>(std::numeric_limits<To_t>::min()) > static_cast<From_t>(-0.5)))
        {
          a.bit_assign(std::numeric_limits<To_t>::min());
          return { std::numeric_limits<To_t>::min(), SIGNED_UNDERFLOW };
        }
      }
    }
    a.bit_assign(static_cast<To_t>(a.as<From_t>()));
    return { a, NO_ERROR };
  }

  /***************** UTILITIES *****************/

  /// @brief QWORD_t binary instruction function type
  using BinaryInst_t = ResultQWORD(*)(QWORD_t, QWORD_t) noexcept;

  /// @brief Helper to avoid casting nullptr to BinaryInst_t type
  inline constexpr BinaryInst_t nullbinary = nullptr;

  /// @brief QWORD_t unary instruction function type
  using UnaryInst_t = ResultQWORD(*)(QWORD_t) noexcept;

  /// @brief Helper to avoid casting nullptr to UnaryInst_t type
  inline constexpr UnaryInst_t nullunary = nullptr;

  constexpr bool isSInt(TypeOp op) noexcept
  {
    return TypeOp::i8_t <= op && op <= TypeOp::i64_t;
  }

  constexpr bool isUInt(TypeOp op) noexcept
  {
    return TypeOp::u8_t <= op && op <= TypeOp::u64_t;
  }

  constexpr bool isInt(TypeOp op) noexcept
  {
    return TypeOp::i8_t <= op && op <= TypeOp::u64_t;
  }

  constexpr bool isFP(TypeOp op) noexcept
  {
    return TypeOp::f32_t == op || TypeOp::f64_t == op;
  }

  constexpr Size sizeofType(TypeOp op) noexcept
  {
    using enum TypeOp;

    switch_no_default (op)
    {
    case i8_t:
    case u8_t:
      return _8bits;
    case i16_t:
    case u16_t:
      return _16bits;
    case i32_t:
    case u32_t:
    case f32_t:
      return _32bits;
    case i64_t:
    case u64_t:
    case f64_t:
      return _64bits;
    }
  }

  namespace details
  {
    COLT_GENERATE_TABLE_FOR(add);
    COLT_GENERATE_TABLE_FOR(sub);
    COLT_GENERATE_TABLE_FOR(mul);
    COLT_GENERATE_TABLE_FOR(div);

    COLT_GENERATE_TABLE_FOR(neg);
    COLT_GENERATE_TABLE_FOR(bit_not);

    COLT_GENERATE_TABLE_FOR(shr);
    COLT_GENERATE_TABLE_FOR(shl);
    COLT_GENERATE_TABLE_FOR(mod);
    COLT_GENERATE_TABLE_FOR(imod);

    COLT_GENERATE_TABLE_FOR(eq);
    COLT_GENERATE_TABLE_FOR(neq);
    COLT_GENERATE_TABLE_FOR(le);
    COLT_GENERATE_TABLE_FOR(ge);
    COLT_GENERATE_TABLE_FOR(leq);
    COLT_GENERATE_TABLE_FOR(geq);

    template<TypeOp Op, TypeOp... Ty>
    consteval auto generate_cnv_row()
    {
      return std::array{ &cnv<Op, Ty>... };
    }

    template<TypeOp... Ty>
    consteval auto generate_cnv()
    {
      return std::array{
        generate_cnv_row<Ty, Ty...>()...
      };
    }
  }

  COLT_TypeOpFn(add);
  COLT_TypeOpFn(sub);
  COLT_TypeOpFn(mul);
  COLT_TypeOpFn(div);

  COLT_SizeFn(mod);
  COLT_SizeFn(imod);
  COLT_NotTemplateFn(bit_and);
  COLT_NotTemplateFn(bit_or);
  COLT_NotTemplateFn(bit_xor);

  COLT_SizeFn(shr);
  COLT_SizeFn(shl);

  COLT_TypeOpFn(eq);
  COLT_TypeOpFn(neq);
  COLT_TypeOpFn(le);
  COLT_TypeOpFn(ge);
  COLT_TypeOpFn(leq);
  COLT_TypeOpFn(geq);

  inline ResultQWORD NT_neg(QWORD_t value, TypeOp type) noexcept
  {
    using enum TypeOp;

    switch_no_default (type)
    {
    case i8_t:
      return neg<_8bits>(value);
    case i16_t:
      return neg<_16bits>(value);
    case i32_t:
      return neg<_32bits>(value);
    case i64_t:
      return neg<_64bits>(value);
    case f32_t:
      return fneg<_32bits>(value);
    case f64_t:
      return fneg<_64bits>(value);
    }
  }

  inline ResultQWORD NT_bit_not(QWORD_t value, TypeOp type) noexcept
  {
    using enum TypeOp;

    switch_no_default(type)
    {
    case i8_t:
    case u8_t:
      return bit_not<_8bits>(value);
    case i16_t:
    case u16_t:
      return bit_not<_16bits>(value);
    case i32_t:
    case u32_t:
      return bit_not<_32bits>(value);
    case i64_t:
    case u64_t:
      return bit_not<_64bits>(value);
    }
  }

  inline ResultQWORD NT_cnv(QWORD_t value, TypeOp from, TypeOp to) noexcept
  {
    static constexpr auto cnv = details::generate_cnv<COLT_TypeOp_PACK>();
    return cnv[(u8)from][(u8)to](value);
  }
}

#endif //!HG_COLT_QWORD_OP