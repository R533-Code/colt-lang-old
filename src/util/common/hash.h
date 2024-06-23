/*****************************************************************/ /**
 * @file   hash.h
 * @brief  Contains hashing utilities used throughout the library.
* Use `hash_value` to hash an object. To add hashing support for
* an object, overload `std::hash` or `clt::hash`.
* `clt::hash` can be overloaded in the same way as `std::hash`,
* using operator().
* If both a `std::hash` and `clt::hash` overloads are found,
* the `clt::hash` will take priority.
 *
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_HASH
#define HG_COLT_HASH

#include <functional>
#include <limits>
#include <cstdint>
#include <utility>
#include <bit>

namespace clt
{
  template<typename T>
  /// @brief Non-overloaded hash struct
  /// @tparam T Non-overloaded type
  struct hash
  {
  };

  template<>
  /// @brief clt::hash overload for bool
  struct hash<bool>
  {
    /// @brief Hashing operator
    /// @param b The value to hash
    /// @return Hash
    constexpr size_t operator()(bool b) const noexcept { return b ? 1231 : 1237; }
  };

  template<>
  /// @brief clt::hash overload for uint32_t
  struct hash<uint32_t>
  {
    /// @brief Hashing operator
    /// @param i The value to hash
    /// @return Hash
    constexpr size_t operator()(uint32_t i) const noexcept
    {
      size_t x = i;
      x        = ((x >> 16) ^ x) * 0x45d9f3b;
      x        = ((x >> 16) ^ x) * 0x45d9f3b;
      x        = (x >> 16) ^ x;
      return x;
    }
  };

  template<>
  /// @brief clt::hash overload for uint64_t
  struct hash<uint64_t>
  {
    /// @brief Hashing operator
    /// @param i The value to hash
    /// @return Hash
    constexpr size_t operator()(uint64_t i) const noexcept
    {
      size_t x = i;
      x        = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
      x        = (x ^ (x >> 27)) * 0x94d049bb133111eb;
      x        = x ^ (x >> 31);
      return x;
    }
  };

  template<>
  /// @brief clt::hash overload for int16_t
  struct hash<int16_t>
  {
    /// @brief Hashing operator
    /// @param i The value to hash
    /// @return Hash
    constexpr size_t operator()(int16_t i) const noexcept
    {
      const auto in = static_cast<uint64_t>(i);
      return clt::hash<uint64_t>{}(in);
    }
  };

  template<>
  /// @brief clt::hash overload for uint16_t
  struct hash<uint16_t>
  {
    /// @brief Hashing operator
    /// @param i The value to hash
    /// @return Hash
    constexpr size_t operator()(uint16_t i) const noexcept
    {
      const auto in = static_cast<uint64_t>(i);
      return hash<uint64_t>{}(in);
    }
  };

  template<>
  /// @brief clt::hash overload for int32_t
  struct hash<int32_t>
  {
    /// @brief Hashing operator
    /// @param i The value to hash
    /// @return Hash
    constexpr size_t operator()(int32_t i) const noexcept
    {
      auto x = static_cast<size_t>(i);
      x      = ((x >> 16) ^ x) * 0x45d9f3b;
      x      = ((x >> 16) ^ x) * 0x45d9f3b;
      x      = (x >> 16) ^ x;
      return x;
    }
  };

  template<>
  /// @brief clt::hash overload for int64_t
  struct hash<int64_t>
  {
    /// @brief Hashing operator
    /// @param i The value to hash
    /// @return Hash
    constexpr size_t operator()(int64_t i) const noexcept
    {
      auto x = static_cast<size_t>(i);
      x      = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
      x      = (x ^ (x >> 27)) * 0x94d049bb133111eb;
      x      = x ^ (x >> 31);
      return x;
    }
  };

  template<>
  /// @brief clt::hash overload for char
  struct hash<char>
  {
    /// @brief Hashing operator
    /// @param i The value to hash
    /// @return Hash
    constexpr size_t operator()(char i) const noexcept
    {
      const auto in = static_cast<uint64_t>(i);
      return hash<uint64_t>{}(in);
    }
  };

  template<>
  /// @brief clt::hash overload for uint8_t
  struct hash<uint8_t>
  {
    /// @brief Hashing operator
    /// @param i The value to hash
    /// @return Hash
    constexpr size_t operator()(uint8_t i) const noexcept
    {
      const auto in = static_cast<uint64_t>(i);
      return hash<uint64_t>{}(in);
    }
  };

  template<>
  /// @brief clt::hash overload for int8_t
  struct hash<int8_t>
  {
    /// @brief Hashing operator
    /// @param i The value to hash
    /// @return Hash
    constexpr size_t operator()(int8_t i) const noexcept
    {
      const auto in = static_cast<uint64_t>(i);
      return hash<uint64_t>{}(in);
    }
  };

  namespace details
  {
    template<typename T>
    /// @brief Performs a xor followed by a shift
    /// @param n The number to xor with its shifted self
    /// @param i By how much to shift
    /// @return n xor (n >> i)
    constexpr T xorshift(T n, int i)
    {
      return n ^ (n >> i);
    }

    /// @brief Distributes a value (useful for hashing)
    /// @param n The value to distribute
    /// @return Distributed value
    constexpr uint32_t distribute(uint32_t n)
    {
      uint32_t p = 0x55555555UL; // pattern of alternating 0 and 1
      uint32_t c = 3423571495UL; // random uneven integer constant
      return c * xorshift(p * xorshift(n, 16), 16);
    }

    /// @brief Distributes a value (useful for hashing)
    /// @param n The value to distribute
    /// @return Distributed value
    constexpr uint64_t distribute(uint64_t n)
    {
      uint64_t p = 0x5555555555555555ULL;   // pattern of alternating 0 and 1
      uint64_t c = 17316035218449499591ULL; // random uneven integer constant
      return c * xorshift(p * xorshift(n, 32), 32);
    }
  } // namespace details

  template<typename T>
  /// @brief clt::hash overload for pointer types
  struct hash<T*>
  {
    /// @brief Hashing operator
    /// @param ptr The value to hash
    /// @return Hash
    constexpr size_t operator()(T* ptr) const noexcept
    {
      auto x = std::bit_cast<std::uintptr_t>(ptr);
      x      = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
      x      = (x ^ (x >> 27)) * 0x94d049bb133111eb;
      x      = x ^ (x >> 31);
      return x;
    }
  };

  template<>
  /// @brief clt::hash overload for float
  struct hash<float>
  {
    /// @brief Hashing operator
    /// @param flt The value to hash
    /// @return Hash
    constexpr size_t operator()(float flt) const noexcept
    {
      auto x = static_cast<size_t>(std::bit_cast<uint32_t>(flt));
      x      = ((x >> 16) ^ x) * 0x45d9f3b;
      x      = ((x >> 16) ^ x) * 0x45d9f3b;
      x      = (x >> 16) ^ x;
      return x;
    }
  };

  template<>
  /// @brief clt::hash overload for double
  struct hash<double>
  {
    /// @brief Hashing operator
    /// @param dbl The value to hash
    /// @return Hash
    constexpr size_t operator()(double dbl) const noexcept
    {
      auto x = std::bit_cast<size_t>(dbl);
      x      = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
      x      = (x ^ (x >> 27)) * 0x94d049bb133111eb;
      x      = x ^ (x >> 31);
      return x;
    }
  };

  namespace meta
  {
    template<typename T, typename = std::void_t<>>
    /// @brief Check if a type implements a std::hash specialization
    /// @tparam T The type to check for
    /// @tparam  SFINAE helper
    struct is_std_hashable
    {
      static constexpr bool value = false;
    };

    template<typename T>
    /// @brief Check if a type implements a std::hash specialization
    /// @tparam T The type to check for
    /// @tparam  SFINAE helper
    struct is_std_hashable<
        T, std::void_t<decltype(std::declval<std::hash<T>>()(std::declval<T>()))>>
    {
      static constexpr bool value = true;
    };

    template<typename T>
    /// @brief Short hand for is_std_hashable<T>::value
    /// @tparam T The type to check for
    inline constexpr bool is_std_hashable_v = is_std_hashable<T>::value;

    template<typename T, typename = std::void_t<>>
    /// @brief Check if a type implements a clt::hash specialization
    /// @tparam T The type to check for
    /// @tparam  SFINAE helper
    struct is_colt_hashable
    {
      static constexpr bool value = false;
    };

    template<typename T>
    /// @brief Check if a type implements a clt::hash specialization
    /// @tparam T The type to check for
    /// @tparam  SFINAE helper
    struct is_colt_hashable<
        T, std::void_t<decltype(std::declval<clt::hash<T>>()(std::declval<T>()))>>
    {
      static constexpr bool value = true;
    };

    template<typename T>
    /// @brief Short hand for is_colt_hashable<T>::value
    /// @tparam T The type to check for
    inline constexpr bool is_colt_hashable_v = is_colt_hashable<T>::value;

    template<typename T>
    /// @brief Check if a type can be either hashed with clt::hash or std::hash
    /// @tparam T The type to check for
    struct is_hashable
    {
      static constexpr bool value = is_std_hashable_v<T> || is_colt_hashable_v<T>;
    };

    template<typename T>
    /// @brief Short hand for is_hashable<T>::value
    /// @tparam T The type to check for
    inline constexpr bool is_hashable_v = is_hashable<T>::value;
  } // namespace meta

  template<typename T>
  /// @brief Hashes an object with clt::hash if implemented, else using std::hash specialization
  /// @tparam T The type to hash
  /// @param obj The object to hash
  /// @return Hash
  constexpr std::size_t hash_value(const T& obj) noexcept
  {
    static_assert(
        meta::is_hashable_v<T>, "Type does not implement clt::hash or std::hash!");
    if constexpr (meta::is_colt_hashable_v<T>)
      return clt::hash<T>{}(obj);
    else
      return std::hash<T>{}(obj);
  }

  /// @brief Combines 2 hashes.
  /// Example Usage:
  /// @code{.cpp}
  /// size_t seed = 0;
  /// seed = hash_combine(seed, hash_value(...));
  /// seed = hash_combine(seed, hash_value(...));
  /// @endcode
  /// @param seed The first hash
  /// @param v The second hash
  /// @return The combined hash
  constexpr std::size_t hash_combine(std::size_t seed, std::size_t v)
  {
    return std::rotl(seed, std::numeric_limits<size_t>::digits / 3)
           ^ details::distribute(v);
  }

  template<typename T1, typename T2>
  /// @brief clt::hash overload for std::pair
  struct hash<std::pair<T1, T2>>
  {
    /// @brief Hashing operator
    /// @param pair The value to hash
    /// @return Hash
    constexpr size_t operator()(const std::pair<T1, T2>& pair) const noexcept
    {
      size_t seed = 0;
      seed        = hash_combine(seed, hash_value(pair.first));
      seed        = hash_combine(seed, hash_value(pair.second));
      return seed;
    }
  };
} // namespace clt

#endif //!HG_COLT_HASH