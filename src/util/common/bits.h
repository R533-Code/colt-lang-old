#ifndef HG_COLT_BITS
#define HG_COLT_BITS

#include "types.h"

namespace clt
{
  /// @brief Generate a bit mask.
  /// As an example, bitmask<u8>(3) -> 0b0000'0111.
  /// @tparam Ty The resulting type
  /// @param one_count The number of ones in the bit mask
  /// @return Bit mask
  template<meta::UnsignedIntegral Ty>
  constexpr Ty bitmask(u8 one_count) noexcept
  {    
    return static_cast<Ty>(-(one_count != 0))
           & (static_cast<Ty>(-1) >> ((sizeof(Ty) * 8) - one_count));
  }

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

  /// @brief Represents a field of 'Bitfields'
  /// @tparam Name The integral type used to identify the bit field
  /// @tparam Size The size of the field
  template<auto Name, size_t Size>
    requires(0 < Size) && (Size <= 64)
  struct Bitfield
  {
    /// @brief Variable to verify that a type is a bit field
    static constexpr bool is_bit_field = true;

    /// @brief The type used to identify the bit field
    using type = decltype(Name);
    /// @brief The value used to identify the bit field
    static constexpr type value = Name;
    /// @brief The size of the bitfield
    static constexpr size_t size = Size;
  };

  /// @brief Portable bit field implementation.
  /// Due to the standard underspecifying bit fields, this class is needed
  /// to make bit fields portable. Only endianess is not taken into account.
  /// @code{.cpp}
  /// // Example usage:
  /// enum class FieldName
  /// {
  ///   Opcode, Payload, Padding
  /// };
  /// using Type = Bitfields<u16,
  ///   Bitfield<FieldName::OpCode, 4>, // ID, bit size
  ///   Bitfield<FieldName::Payload, 8>,
  ///   Bitfield<FieldName::Padding, 4>>;
  /// @endcode
  /// 
  /// @tparam Field0 The first field
  /// @tparam ...Fields The 
  /// @tparam Ty 
  template<meta::UnsignedIntegral Ty, typename Field0, typename... Fields>
  class Bitfields
  {
    static_assert(
        (Field0::is_bit_field && ... && Fields::is_bit_field),
        "All the underlying type used to identify the field must be the same");
    static_assert(
        (Field0::size + ... + Fields::size) == sizeof(Ty) * 8,
        "The sum of all the bitfields' size must fill all the bits of the "
        "underlying type!");
    static_assert(
        meta::are_all_same<typename Field0::type, typename Fields::type...>,
        "All the underlying type used to identify the field must be the same");

    /// @brief The type of the field ID
    using index_t = Field0::type;
    /// @brief The number of fields
    static constexpr size_t field_count = 1 + sizeof...(Fields);

    /// @brief The underlying storage
    Ty storage;

    /// @brief Returns the informations about the field of name 'index'
    /// @tparam index The field name
    /// @return Pair containing the offset to the field, and size of the field
    template<auto index> requires std::same_as<index_t, decltype(index)>
    static consteval std::pair<u64, u64> field_info() noexcept
    {
      std::pair<index_t, size_t> array[field_count] = {
          std::pair{Field0::value, Field0::size},
          std::pair{Fields::value, Fields::size}...};
      u64 offset = sizeof(Ty) * 8;
      for (size_t i = 0; i < field_count; i++)
      {
        offset -= array[i].second;
        if (array[i].first == index)
          return {offset, array[i].second};
      }
      assert_true("Invalid field name!", false);
    }

  public:
    /// @brief Constructs an empty Bitfields (set to all zeros)
    constexpr Bitfields() noexcept
        : storage(0)
    {
    }
    /// @brief Constructs a Bitfields
    /// @param value The value to initialize to
    constexpr Bitfields(Ty value) noexcept
        : storage(value)
    {
    }

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(Bitfields);

    /// @brief Returns the bit field of ID 'index'.
    /// If this method produces an 'expression cannot be constant evaluated'
    /// or such, this means the ID is invalid.
    /// @tparam index The ID of the field
    /// @return The value stored in the bit field
    template<auto index>
      requires std::same_as<index_t, decltype(index)>
    constexpr Ty get() const noexcept
    {
      static constexpr auto [offset, size] = field_info<index>();
      return (storage >> offset) & bitmask<Ty>(size);
    }

    /// @brief Sets the value of the bit field of ID 'index'
    /// This will only keep as much bits from 'value' as the field can store.
    /// If this method produces an 'expression cannot be constant evaluated'
    /// or such, this means the ID is invalid.
    /// @tparam index The ID of the field
    /// @param value The value to store
    template<auto index>
      requires std::same_as<index_t, decltype(index)>
    constexpr void set(Ty value) noexcept
    {
      static constexpr auto [offset, size] = field_info<index>();
      // Set the bit whose value to modify to 0
      storage &= ~(bitmask<Ty>(size) << offset);
      // OR the new bits
      storage |= ((value & bitmask<Ty>(size)) << offset);
    }

    /// @brief Returns the underlying value
    /// @return The underlying value
    constexpr Ty value() const noexcept { return storage; }
  };
}

#endif // !HG_COLT_BITS
