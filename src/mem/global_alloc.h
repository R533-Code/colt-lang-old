/*****************************************************************//**
 * @file   global_alloc.h
 * @brief  Contains the global allocator, and descriptions helpers.
 * 
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_GLOBAL_ALLOC
#define HG_COLT_GLOBAL_ALLOC

#include "composable_alloc.h"

namespace clt::mem
{
  /// @brief The global allocator
  inline AbortOnNULLAllocator<
    Segregator<1_KiB,
    ThreadSafeAllocator<Segregator<256, FreeList<StackAllocator<8_KiB, 16>, 16_B, 256_B, 32>, FreeList<Mallocator, 256_B, 1_KiB, 32>>>,
    ThreadSafeAllocator<FreeList<Mallocator, 1_KiB, 4_KiB, 32>>
    >> GlobalAllocator;

  /// @brief Allocate a block of memory through the global allocator
  /// @param sz The size of the block
  /// @return A MemBlock that is NEVER null
  static MemBlock global_alloc(ByteSize<Byte> sz) noexcept
  {
    return GlobalAllocator.alloc(sz);
  }

  /// @brief Deallocate a block of memory through the global allocator.
  /// This function can accept a 'nullblk' even though 'alloc' never returns a 'nullblk'.
  /// @param blk The block to deallocate
  static void global_dealloc(MemBlock blk) noexcept
  {
    if (!blk.is_null())
      GlobalAllocator.dealloc(blk);
  }

  /// @brief Registers a function to call if memory could not be allocated.
  /// This function will be called before aborting the app.
  /// @param fn The function to register
  static void global_on_null(void(*fn)() noexcept) noexcept
  {
    GlobalAllocator.register_on_null(fn);
  }

  /// @brief Function pointer to an allocation function
  using AllocFn   = MemBlock(*)(ByteSize<Byte>) noexcept;
  /// @brief Function pointer to a deallocation function
  using DeallocFn = void(*)(MemBlock) noexcept;
  /// @brief Function pointer to a reallocation function
  using ReallocFn = bool(*)(MemBlock, ByteSize<Byte>) noexcept;
  /// @brief Function pointer to an expanding function
  using ExpandFn  = bool(*)(MemBlock, ByteSize<Byte>) noexcept;
  /// @brief Function pointer to an owning function
  using OwnFn     = bool(*)(MemBlock) noexcept;
  
  template<AllocFn A, DeallocFn D, ReallocFn R, ExpandFn E, OwnFn O>
  /// @brief Describes a global allocator
  struct AllocatorDescription
  {
    static_assert(A != nullptr && D != nullptr,
      "An allocator at least has an allocation and deallocation function!");

    static constexpr bool is_allocator_description = true;

    /// @brief Function pointer to the allocation function (never null)
    static constexpr AllocFn alloc_fn = A;
    /// @brief Function pointer to the deallocation function (never null)
    static constexpr DeallocFn dealloc_fn = D;
    /// @brief Function pointer to the reallocation function (can be null)
    static constexpr ReallocFn realloc_fn = R;
    /// @brief Function pointer to the expanding function (can be null)
    static constexpr ExpandFn expand_fn = E;
    /// @brief Function pointer to the owning function (can be null)
    static constexpr OwnFn own_fn = O;
  };

  /// @brief Description of the GlobalAllocator
  inline constexpr AllocatorDescription<&clt::mem::global_alloc, &clt::mem::global_dealloc, nullptr, nullptr, nullptr> GlobalAllocatorDescription{};

  /// @brief Tag type for asking
  template<typename T>
  struct LocalAllocatorTag
  {
    /// @brief The type of the allocator
    using allocator_type = T;
  };

  template<typename T>
  /// @brief Helper tag for local allocators used in data structure
  /// @tparam T The type of the allocator
  inline constexpr LocalAllocatorTag<T> LocalAllocator;
}

namespace clt::meta
{
  template<auto T>
  /// @brief A LocalAllocator is an a LocalAllocatorTag<...>
  concept LocalAllocator = Allocator<std::decay_t<typename decltype(T)::allocator_type>>;
  
  template<auto T>
  /// @brief A Global allocator is a AllocatorDescription
  concept GlobalAllocator = (decltype(T)::is_allocator_description == true);

  template<auto T>
  /// @brief An allocator value is either a LocalAllocator or GlobalAllocator
  concept AllocatorScope = GlobalAllocator<T> || LocalAllocator<T>;
}

namespace clt::mem
{
  template<auto ALLOCATOR>
    requires meta::AllocatorScope<ALLOCATOR>
  /// @brief Non-specialized class helper.
  /// To simplify having a data-structure that can either use a global allocator
  /// or a local one, the data-structure takes as a template parameter
  /// a meta::AllocatorScope<ALLOCATOR>, and inherits from allocator_ref<ALLOCATOR>.
  /// Constructor then have to check (using concepts) if ALLOCATOR is local or global.
  /// If local, then the constructor NEEDS to take in a reference to the allocator,
  /// and initialize 'allocator_ref' with that reference.
  /// To allocate and deallocate, use allocator_ref<ALLOCATOR>.alloc()/dealloc().
  struct allocator_ref {};

  template<auto ALLOCATOR>
    requires (meta::LocalAllocator<ALLOCATOR>)
  /// @brief Overload for local allocators: Contains a reference to the allocator.
  struct allocator_ref<ALLOCATOR>
  {
    /// @brief The allocator type
    using alloc_t = typename decltype(ALLOCATOR)::allocator_type;    

    /// @brief The reference to the allocator to use
    alloc_t* ref;

    // No default constructor
    allocator_ref() = delete;
    
    /// @brief Constructor
    /// @param ref Reference to the allocator
    constexpr allocator_ref(alloc_t& ref) noexcept
      : ref(&ref) {}

    constexpr allocator_ref(const allocator_ref&)             noexcept = default;
    constexpr allocator_ref& operator=(const allocator_ref&)   noexcept = default;
    constexpr allocator_ref(allocator_ref&&)                  noexcept = default;
    constexpr allocator_ref& operator=(allocator_ref&&)        noexcept = default;

    /// @brief Allocates a MemBlock through the reference to the allocator
    /// @param size The size of the block
    /// @return Result of allocation
    constexpr MemBlock alloc(ByteSize<Byte> size) noexcept
    {
      return ref->alloc(size);
    }

    /// @brief Deallocates a MemBlock through the reference to the allocator
    /// @param blk The block to deallocate
    constexpr void dealloc(MemBlock blk) noexcept
    {
      return ref->dealloc(blk);
    }
  };

  template<meta::Allocator alloc>
  /// @brief CTAD helper
  allocator_ref(alloc&)->allocator_ref<mem::LocalAllocator<alloc>>;

  template<auto ALLOCATOR>
    requires (meta::GlobalAllocator<ALLOCATOR>)
  /// @brief Overload for global allocators: uses information in the AllocatorDescription to allocate/deallocate.
  struct allocator_ref<ALLOCATOR>
  {
    /// @brief Allocates a MemBlock through the global allocator
    /// @param size The size of the block
    /// @return Result of allocation
    constexpr MemBlock alloc(ByteSize<Byte> size) noexcept
    {
      return (*decltype(ALLOCATOR)::alloc_fn)(size);
    }

    /// @brief Deallocates a MemBlock through the global allocator
    /// @param blk The block to deallocate
    constexpr void dealloc(MemBlock blk) noexcept
    {
      return (*decltype(ALLOCATOR)::dealloc_fn)(blk);
    }
  };
}


#endif //!HG_COLT_GLOBAL_ALLOC