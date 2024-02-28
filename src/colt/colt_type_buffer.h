#ifndef HG_COLTC_TYPE_BUFFER
#define HG_COLTC_TYPE_BUFFER

#include "colt_type.h"
#include "structs/set.h"
#include "colt_type_token.h"

namespace clt::lng
{
  /// @brief Class responsible of storing types.
  class TypeBuffer
  {
    /// @brief Set of types
    IndexedSet<TypeVariant> type_map{};

#ifdef COLT_DEBUG
    static std::atomic<u32> ID_GENERATOR;

    u32 buffer_id;
#endif // COLT_DEBUG

    /// @brief Check if a Token is owned by the current TokenBuffer
    constexpr void owns(TypeToken tkn) const noexcept
    {
#ifdef COLT_DEBUG
      if constexpr (isDebugBuild())
        assert_true("Token is not owned by this TypeBuffer!", tkn.buffer_id == buffer_id);
#endif // COLT_DEBUG
    }

    /// @brief Returns the next token to save
    /// @return The token to save
    constexpr TypeToken createToken(size_t sz) const noexcept
    {
      assert_true("Integer overflow!", sz <= std::numeric_limits<u32>::max());

#ifdef COLT_DEBUG
      return TypeToken(static_cast<u32>(sz), buffer_id);
#else
      return TypeToken(static_cast<u32>(sz));
#endif // COLT_DEBUG
    }

  public:
    /// @brief Default constructor
    TypeBuffer() noexcept
    {
#ifdef COLT_DEBUG
      if constexpr (isDebugBuild())
        buffer_id = ID_GENERATOR.fetch_add(1, std::memory_order_acq_rel);
#endif // COLT_DEBUG
    }

    /// @brief Saves a type and return its index number
    /// @param variant The type to save
    /// @return The TypeToken representing the type
    TypeToken addType(const TypeVariant& variant) noexcept
    {
      auto [pair, insert] = type_map.insert(variant);
      return createToken(pair);
    }

    /// @brief Get a type using its token.
    /// The reference is valid as long as addType was not called!
    /// @param tkn The token whose type to return
    /// @return The type represented by 'tkn'
    const TypeVariant& getType(TypeToken tkn) const noexcept
    {
      return type_map.internal_list()[tkn.type_index];
    }
  };
}

#endif // !HG_COLTC_TYPE_BUFFER
