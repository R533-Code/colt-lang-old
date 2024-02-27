#ifndef HG_COLTC_TYPE_BUFFER
#define HG_COLTC_TYPE_BUFFER

#include "colt_type.h"
#include "structs/map.h"

namespace clt::lng
{
  class TypeBuffer
  {
    Map<TypeVariant, TypeToken> type_map{};
    Vector<const TypeVariant*> type_vec{};

#ifdef COLT_DEBUG
    /// @brief 
    static std::atomic<u32> ID_GENERATOR;

    u32 buffer_id;
#endif // COLT_DEBUG

    /// @brief Check if a Token is owned by the current TokenBuffer
    constexpr void owns(TypeToken tkn) const noexcept
    {
#ifdef COLT_DEBUG
      if constexpr (isDebugBuild())
        assert_true("Token is not owned by this TokenBuffer!", tkn.buffer_id == buffer_id);
#endif // COLT_DEBUG
    }

    constexpr TypeToken getNext() const noexcept
    {
#ifdef COLT_DEBUG
      return TypeToken(type_map.size(), buffer_id);
#else
      return TypeToken(type_map.size());
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
    TypeToken addType(TypeVariant&& variant) noexcept
    {
      auto pair = type_map.insert(std::move(variant), getNext());
      if (pair.second == InsertionResult::SUCCESS)
        type_vec.push_back(&pair.first->first);
      return pair.first->second;
    }

    const TypeVariant& getType(TypeToken tkn) const noexcept
    {
      return *type_vec[tkn.type_index];
    }
  };
}

#endif // !HG_COLTC_TYPE_BUFFER
