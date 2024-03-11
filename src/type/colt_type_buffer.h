/*****************************************************************//**
 * @file   colt_type_buffer.h
 * @brief  Contains TypeBuffer, responsible for storing types.
 * 
 * @author RPC
 * @date   March 2024
 *********************************************************************/
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

    /// @brief Saves a built-in type
    /// @param id The type ID
    /// @return The TypeToken representing the type
    TypeToken addBuiltin(BuiltinID id) noexcept
    {
      return addType(ColtBuiltinTypeTable[static_cast<u8>(id)]);
    }

    /// @brief Saves a pointer to a type
    /// @param to The type pointed to
    /// @return The TypeToken representing the pointer
    TypeToken addPtr(TypeToken to) noexcept
    {
      return addType(make_coltc_type<PtrType>(to));
    }
    
    /// @brief Saves a mutable pointer to a type
    /// @param to The type pointed to
    /// @return The TypeToken representing the mutable pointer
    TypeToken addMutPtr(TypeToken to) noexcept
    {
      return addType(make_coltc_type<MutPtrType>(to));
    }
    
    /// @brief Saves a pointer to a type
    /// @param to The type pointed to
    /// @return The TypeToken representing the pointer
    TypeToken addOpaquePtr() noexcept
    {
      return addType(make_coltc_type<OpaquePtrType>());
    }
    
    /// @brief Saves a mutable pointer to a type
    /// @param to The type pointed to
    /// @return The TypeToken representing the mutable pointer
    TypeToken addMutOpaquePtr() noexcept
    {
      return addType(make_coltc_type<MutOpaquePtrType>());
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
