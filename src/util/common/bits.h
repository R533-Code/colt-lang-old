#ifndef HG_COLT_BITS
#define HG_COLT_BITS

#include "types.h"

namespace clt
{
  /// @brief Swaps the bytes of an integer (opposite endianness).
  /// @tparam T The unsigned integer type
  /// @param a The value whose bytes to swap
  /// @return The integer in the opposite endianness
  template<meta::UnsignedIntegral T>
  constexpr T byteswap(T a) noexcept
  {
    if constexpr (sizeof(T) == 1)
      return a;
#ifdef COLT_MSVC
    if constexpr (sizeof(T) == 2)
      return _byteswap_ushort(a);
    if constexpr (sizeof(T) == 4)
      return _byteswap_ulong(a);
    if constexpr (sizeof(T) == 8)
      return _byteswap_uint64(a);
#elif defined(COLT_GNU) || defined(COLT_CLANG)
    if constexpr (sizeof(T) == 2)
      return __builtin_bswap16(a);
    if constexpr (sizeof(T) == 4)
      return __builtin_bswap32(a);
    if constexpr (sizeof(T) == 8)
      return __builtin_bswap64(a);
#else
    // Undefined behavior...
    union
    {
      u8 buffer[sizeof(T)];
      T u;
    } source, dest;

    source.u = u;

    for (size_t k = 0; k < sizeof(T); k++)
      dest.u8[k] = source.buffer[sizeof(T) - k - 1];

    return dest.u;
#endif // COLT_MSVC
  }

  /// @brief Converts an unsigned integer from host endianness to little endian.
  /// This function is a no-op if the current host is little endian.
  /// @tparam T The unsigned integer type
  /// @param a The value to convert
  /// @return The integer encoded as little endian
  template<meta::UnsignedIntegral T>
  constexpr T htol(T a) noexcept
  {
    using enum std::endian;
    static_assert(native == little || native == big, "Unknown endianness!");
    static_assert(sizeof(T) <= 8, "Invalid integer size!");

    if constexpr (sizeof(T) == 1 || native == little)
      return a;
    else
      return byteswap(a);
  }

  /// @brief Converts an unsigned integer from host endianness to big endian.
  /// This function is a no-op if the current host is big endian.
  /// @tparam T The unsigned integer type
  /// @param a The value to convert
  /// @return The integer encoded as big endian
  template<meta::UnsignedIntegral T>
  constexpr T htob(T a) noexcept
  {
    using enum std::endian;
    static_assert(native == little || native == big, "Unknown endianness!");
    static_assert(sizeof(T) <= 8, "Invalid integer size!");

    if constexpr (sizeof(T) == 1 || native == big)
      return a;
    else
      return byteswap(a);
  }

  /// @brief Converts an unsigned integer from little endian host endianness.
  /// This function is a no-op if the current host is little endian.
  /// @tparam T The unsigned integer type
  /// @param a The value to convert
  /// @return The integer encoded as host endianness
  template<meta::UnsignedIntegral T>
  constexpr T ltoh(T a) noexcept
  {
    using enum std::endian;
    static_assert(native == little || native == big, "Unknown endianness!");
    static_assert(sizeof(T) <= 8, "Invalid integer size!");

    if constexpr (sizeof(T) == 1 || native == little)
      return a;
    else
      return byteswap(a);
  }

  /// @brief Converts an unsigned integer from big endian to host endianness.
  /// This function is a no-op if the current host is big endian.
  /// @tparam T The unsigned integer type
  /// @param a The value to convert
  /// @return The integer encoded as host endianness
  template<meta::UnsignedIntegral T>
  constexpr T btoh(T a) noexcept
  {
    using enum std::endian;
    static_assert(native == little || native == big, "Unknown endianness!");
    static_assert(sizeof(T) <= 8, "Invalid integer size!");

    if constexpr (sizeof(T) == 1 || native == big)
      return a;
    else
      return byteswap(a);
  }

  /// @brief Sign extends a number represented by 'n' bits
  /// @tparam T The underlying type to sign extend
  /// @param value The value (represented by 'n' bits)
  /// @param n The number of bit from which to sign extend
  /// @return The sign extended integer
  template<meta::UnsignedIntegral T>
  constexpr std::make_signed_t<T> sign_extend(T value, u8 n)
  {
    assert_true("Invalid bit count!", n > 0 && n < sizeof(T) * 8);
    T sign = (1 << (n - 1)) & value;
    T mask = ((~0U) >> (n - 1)) << (n - 1);
    if (sign != 0)
      value |= mask;
    else
      value &= ~mask;
    return static_cast<std::make_signed_t<T>>(value);
  }
}

#endif // !HG_COLT_BITS
