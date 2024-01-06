/*****************************************************************//**
 * @file   allocator_traits.h
 * @brief  Contains concepts to identify allocators.
 * 
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_ALLOCATOR_TRAITS
#define HG_COLT_ALLOCATOR_TRAITS

#include <type_traits>
#include <concepts>

#include "block.h"

namespace clt::meta
{
  template<typename T>
  /// @brief An allocator provides a 'alloc()' and 'dealloc()' method
  concept Allocator = (std::same_as<std::decay_t<decltype(T::alignment)>, u64>) &&
    requires (T t, mem::MemBlock b, ByteSize<Byte> sz)
  {
    { t.alloc(sz) } -> std::same_as<mem::MemBlock>;
    { t.dealloc(b) } -> std::same_as<void>;
  };

  template<typename T>
  /// @brief An allocator provides a 'alloc()', 'dealloc()' and 'owns()' method
  concept OwningAllocator = Allocator<T> && requires (T t, mem::MemBlock b)
  {
    { t.owns(b) } -> std::same_as<bool>;
  };

  template<typename T>
  /// @brief An allocator provides a 'alloc()', 'dealloc()' and 'owns()' method
  concept ExpandingAllocator = Allocator<T> && requires (T t, mem::MemBlock b, ByteSize<Byte> delta)
  {
    { t.expand(b, delta) } -> std::same_as<bool>;
  };

  template<typename T>
  /// @brief An allocator provides a 'alloc()', 'dealloc()' and 'owns()' method
  concept ReallocatableAllocator = Allocator<T> && requires (T t, mem::MemBlock b, ByteSize<Byte> delta)
  {
    { t.realloc(b, delta) } -> std::same_as<bool>;
  };
}

#endif //!HG_COLT_ALLOCATOR_TRAITS