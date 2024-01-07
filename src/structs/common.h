/*****************************************************************//**
 * @file   common.h
 * @brief  Contains common helpers for data-structures.
 * 
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_HELPER
#define HG_COLT_HELPER

#include <cstring>
#include <type_traits>
#include <fmt/format.h>

#include "meta/meta_traits.h"
#include "meta/meta_maths.h"
#include "util/assertions.h"
#include "util/types.h"

namespace clt::details
{
  template<typename T>
  /// @brief Moves and destructs 'count' objects from a memory location to another.
  /// @tparam T The type of the object to move and destruct
  /// @param from Pointer to the objects to move then destruct
  /// @param to Pointer to where to move constructs the objects
  /// @param count The number of objects to move constructs
  constexpr void contiguous_destructive_move(T* from, T* to, size_t count)
    noexcept(std::is_nothrow_move_constructible_v<T>
      && std::is_nothrow_destructible_v<T>)
  {
    assert_true("Invalid arguments!", clt::abs(from - to) >= count);

    if (!std::is_constant_evaluated())
    {
      if constexpr (std::is_trivially_move_constructible_v<T> && std::is_trivially_destructible_v<T>)
      {
        std::memcpy(to, from, count * sizeof(T));
        return;
      }
    }
    for (size_t i = 0; i < count; i++)
    {
      new(to + i) T(std::move(from[i]));
      from[i].~T();
    }
  }

  template<typename T>
  /// @brief Moves 'count' objects from a memory location to another.
  /// @tparam T The type to move
  /// @param from Pointer to the objects to move
  /// @param to Pointer to where to move constructs the objects
  /// @param count The number of objects to move
  constexpr void contiguous_move(T* from, T* to, size_t count)
    noexcept(std::is_nothrow_move_constructible_v<T>)
  {
    assert_true("Invalid arguments!", clt::abs(from - to) >= count);
    if (!std::is_constant_evaluated())
    {
      if constexpr (std::is_trivially_move_constructible_v<T>)
      {
        std::memcpy(to, from, count * sizeof(T));
        return;
      }
    }
    for (size_t i = 0; i < count; i++)
      new(to + i) T(std::move(from[i]));
  }

  template<typename T, typename... Args>
  /// @brief Constructs 'count' objects 'where' by forwarding 'args' to the constructor
  /// @tparam T The type to construct
  /// @tparam ...Args The parameter pack
  /// @param where Pointer to where to constructs the objects
  /// @param count The count of objects to construct
  /// @param ...args The argument pack
  constexpr void contiguous_construct(T* where, size_t count, Args&&... args)
    noexcept(std::is_nothrow_constructible_v<T, Args...>)
  {
    for (size_t i = 0; i < count; i++)
      new(where + i) T(std::forward<Args>(args)...);
  }

  template<typename T>
  /// @brief Copies 'count' objects from a memory location to another one.
  /// @tparam T The type to copy
  /// @param from Pointer to where to copy the objects from
  /// @param to Pointer to where to copy constructs the objects
  /// @param count The number of objects to copy constructs
  inline void contiguous_copy(const T* from, T* to, size_t count)
    noexcept(std::is_nothrow_copy_constructible_v<T>)
  {
    if (!std::is_constant_evaluated())
    {
      if constexpr (std::is_trivially_move_constructible_v<T> && std::is_trivially_destructible_v<T>)
      {
        std::memcpy(to, from, count * sizeof(T));
        return;
      }
    }
    for (size_t i = 0; i < count; i++)
      new(to + i) T(from[i]);
  }

  template<typename T>
  /// @brief Destroys 'count' objects from 'begin'
  /// @tparam T The type to destroy
  /// @param begin Pointer to the objects to destroy
  /// @param count The number of objects to destroy
  inline void contiguous_destruct(T* begin, size_t count)
    noexcept(std::is_nothrow_destructible_v<T>)
  {
    if constexpr (!std::is_trivially_destructible_v<T>)
    {
      for (size_t i = 0; i < count; i++)
        begin[i].~T();
    }
  }

  /// @brief Specifies the state of a map slot.
  /// An ACTIVE sentinel holds 7 bits worth of hash.
  /// An ACTIVE sentinel is one whose highest bit is 0.
  /// An EMPTY sentinel specifies that the slot is empty.
  /// A DELETED sentinel specifies that find should continue searching past that object.
  enum KeySentinel
    : uint8_t
  {
    ACTIVE = 0b00000000,
    EMPTY = 0b10000000,
    DELETED = 0b10000001,
  };

  /// @brief Creates an ACTIVE sentinel holding the 7th lowest bits of 'hash'
  /// @param hash The hash whose 7th lowest bits to encode
  /// @return An ACTIVE sentinel
  constexpr KeySentinel create_active_sentinel(size_t hash) noexcept
  {
    return static_cast<KeySentinel>(hash & 0b01111111);
  }

  /// @brief Check if a sentinel is ACTIVE
  /// @param key The sentinel to check
  /// @return True if the sentinel represents an ACTIVE sentinel
  constexpr bool is_sentinel_active(KeySentinel key) noexcept
  {
    return (key & 0b10000000) == 0;
  }

  /// @brief Check if a sentinel is EMPTY or DELETED
  /// @param key The sentinel to check
  /// @return True if the sentinel represents an ACTIVE sentinel
  constexpr bool is_sentinel_empty_or_deleted(KeySentinel key) noexcept
  {
    return (key & 0b10000000) != 0;
  }

  /// @brief Check if a sentinel is EMPTY
  /// @param key The sentinel to check
  /// @return True if the sentinel equal EMPTY
  constexpr bool is_sentinel_empty(KeySentinel key) noexcept
  {
    return key == EMPTY;
  }

  /// @brief Check if a sentinel is DELETED
  /// @param key The sentinel to check
  /// @return True if the sentinel equal DELETED
  constexpr bool is_sentinel_deleted(KeySentinel key) noexcept
  {
    return key == DELETED;
  }

  /// @brief Check if a hash and a sentinel are equal.
  /// The check performed only compares the lower 7-bits of the hash.
  /// @pre is_sentinel_active(key).
  /// @param key The sentinel to check for
  /// @param hash The hash whose 7 lowest bits to compare with
  /// @return True if the lowest 7-bits of the hash matches the sentinel
  constexpr bool is_sentinel_equal(KeySentinel key, size_t hash) noexcept
  {
    assert_true("Sentinel must be active!", is_sentinel_active(key));
    return (hash & 0b01111111) == (key & 0b01111111);
  }

  /// @brief Increments a probing index, faster than a modulo operation.
  /// @pre precondition, the index must be in the range [0, mod).
  /// @param prob The probing index to increment
  /// @return The incremented index
  constexpr size_t advance_prob(size_t prob, size_t mod) noexcept
  {
    //If this asserts then the optimization should be checked
    assert_true("Verify optimization!", (prob + 1) % mod == ((prob + 1) * (prob + 1 != mod)));
    return  (prob + 1) * static_cast<size_t>(prob + 1 != mod);
  }
}

namespace clt
{
  /// @brief Represents the result of an insert/insert_or_assign operation
  enum class InsertionResult
    : u8
  {
    /// @brief Insertion successful
    SUCCESS,
    /// @brief The key already exists, and nothing was performed
    EXISTS,
    /// @brief Performed an assignment rather than an insertion
    ASSIGNED
  };
}

#endif //!HG_COLT_HELPER