/*****************************************************************//**
 * @file   unique_ptr.h
 * @brief  UniquePtr supporting custom Colt allocators.
 * 
 * @author RPC
 * @date   February 2024
 *********************************************************************/
#ifndef HG_COLT_UNIQUE_PTR
#define HG_COLT_UNIQUE_PTR

#include "mem/global_alloc.h"
#include "common/types.h"

namespace clt
{
  template<typename T, auto ALLOCATOR = mem::GlobalAllocatorDescription>
  /// @brief Unique pointer that automatically frees an allocated resource.
  /// A unique pointer is movable but not copyable, which makes it own its resource.
  class UniquePtr
  {
    static_assert(meta::AllocatorScope<ALLOCATOR>, "Invalid allocator!");

    /// @brief True if the allocator is global
    static constexpr bool is_global = meta::GlobalAllocator<ALLOCATOR>;
    /// @brief True if the allocator is local (we need a reference to it)
    static constexpr bool is_local = meta::LocalAllocator<ALLOCATOR>;

    /// @brief Type of the allocator
    using Allocator = mem::allocator_ref<ALLOCATOR>;

    /// @brief The allocator used for allocation/deallocation
    [[no_unique_address]]
    Allocator allocator;
    /// @brief The memory block owned
    mem::MemBlock blk = {};

  public:
    template<typename U, auto ALL>
    friend class UniquePtr;

    //Non-copyable
    UniquePtr(const UniquePtr&) = delete;
    //Non-copy-assignable
    UniquePtr& operator=(const UniquePtr&) = delete;
    
    /// @brief Constructs an empty UniquePtr
    constexpr UniquePtr() noexcept requires is_global = default;


    template<meta::Allocator AllocT> requires is_local
    /// @brief Constructs an empty UniquePtr
    /// @param alloc The allocator
    constexpr UniquePtr(AllocT& alloc) noexcept
      : allocator(alloc) {}
    
    /// @brief Constructs a UniquePtr from a MemBlock.
    /// On debug, sets the block to nullblk.
    /// @param blk The block to use
    constexpr UniquePtr(meta::for_debug_for_release_t<mem::MemBlock&, mem::MemBlock> blk) noexcept requires is_global
      : blk(blk)
    {
      if constexpr (is_debug_build())
        blk = mem::nullblk;
    }

    /// @brief Constructs a UniquePtr from a MemBlock.
    /// On debug, sets the block to nullblk.
    /// @param alloc The allocator
    /// @param blk The block to use
    template<meta::Allocator AllocT> requires is_local
    constexpr UniquePtr(AllocT& alloc, meta::for_debug_for_release_t<mem::MemBlock&, mem::MemBlock> blk) noexcept
      : allocator(alloc), blk(blk)
    {
      if constexpr (is_debug_build())
        blk = mem::nullblk;
    }
    
    /// @brief Move constructor, steals the resources of 'u'
    /// @param u The unique pointer whose resources to steal
    constexpr UniquePtr(UniquePtr<T, ALLOCATOR>&& u) noexcept
      : allocator(u.allocator), blk(std::exchange(u.blk, mem::nullblk)) {}
    
    template<typename U> requires std::convertible_to<U*, T*>
    /// @brief Converting move constructor, steals the resources of 'u'
    /// @tparam U Type that is convertible to T
    /// @param u The unique pointer whose resources to steal
    constexpr UniquePtr(UniquePtr<U, ALLOCATOR>&& u) noexcept
      : allocator(u.allocator), blk(std::exchange(u.blk, mem::nullblk)) {}
    
    /// /// @brief Destructor, frees resource
    constexpr ~UniquePtr() noexcept(std::is_nothrow_destructible_v<T>)
    {
      if (!blk.is_null())
      {
        ON_SCOPE_EXIT{
          allocator.dealloc(blk);
        };
        //run destructor
        static_cast<T*>(blk.ptr())->~T();
      }
    }

    /// @brief Move assignment operator
    /// @param r The pointer whose content to swap with
    /// @return Self
    UniquePtr& operator=(UniquePtr&& r) noexcept
    {
      if constexpr (is_local)
        std::swap(r.allocator, allocator);
      std::swap(blk, r.blk);
      return *this;
    }

    template<typename U> requires std::convertible_to<U*, T*>
    /// @brief Move assignment operator
    /// @param r The pointer whose content to swap with
    /// @return Self
    UniquePtr& operator=(UniquePtr<U, ALLOCATOR>&& u) noexcept
    {
      if constexpr (is_local)
        std::swap(u.allocator, allocator);
      std::swap(blk, u.blk);
      return *this;
    }

    /// @brief Releases the owned block
    /// @return The owned block
    mem::MemBlock release() noexcept
    {
      return std::exchange(blk, mem::nullblk);
    }

    /// @brief Frees resources and take ownership of 'ptr'
    /// @param ptr The block whose ownership to take
    void reset(mem::MemBlock ptr = mem::nullblk) noexcept(std::is_nothrow_destructible_v<T>)
    {
      auto cpy = blk;
      blk = ptr;
      if (!cpy.is_null())
      {
        ON_SCOPE_EXIT{
          allocator.dealloc(cpy);
        };
        //run destructor
        static_cast<T*>(cpy.ptr())->~T();
      }
    }

    /// @brief Returns a reference to the owned MemBlock
    /// @return Reference to the owned MemBlock
    const mem::MemBlock& get() const noexcept
    {
      return blk;
    }

    /// @brief Check if the owned MemBlock is null
    /// @return True if null
    constexpr bool is_null() const noexcept { return get().is_null(); }

    /// @brief Check if the owned MemBlock is not null
    explicit constexpr operator bool() const noexcept { return !is_null(); }

    /// @brief Dereference operator
    /// @return Dereferences pointer
    std::add_lvalue_reference_t<T> operator*() const noexcept(noexcept(*std::declval<T*>()))
    {
      assert_true("unique_ptr was null!", !is_null());
      return *static_cast<T*>(blk.ptr());
    }

    /// @brief Dereference operator
    /// @return Dereferences pointer
    T* operator->() const noexcept
    {
      assert_true("unique_ptr was null!", !is_null());
      return *static_cast<T*>(blk.ptr());
    }
  };

  template<typename T, auto ALLOCATOR = mem::GlobalAllocatorDescription, typename... Args>
  /// @brief Constructs a UniquePtr using a global allocator
  /// @tparam T The type to construct
  /// @param args... The arguments to forward to the constructor
  /// @return UniquePtr
  UniquePtr<T, ALLOCATOR> make_unique(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
  {
    using Allocator = mem::allocator_ref<ALLOCATOR>;

    Allocator allocator;
    auto blk = allocator.alloc(sizeof(T));
    if constexpr (std::is_nothrow_constructible_v<T, Args...>)
    {
      new(blk.ptr()) T(std::forward<Args>(args)...);
    }
    else
    {
      try
      {
        new(blk.ptr()) T(std::forward<Args>(args)...);
      }
      catch (...)
      {
        allocator.dealloc(blk);
        throw;
      }
    }
    return UniquePtr<T, ALLOCATOR>(blk);
  }

  template<typename T, meta::Allocator Alloc, typename... Args>
  /// @brief Constructs a UniquePtr using a local allocator
  /// @tparam T The type to construct
  /// @param ref The local allocator
  /// @param args... The arguments to forward to the constructor
  /// @return UniquePtr
  constexpr UniquePtr<T, mem::LocalAllocator<Alloc>> make_local_unique(Alloc& ref, Args&&... args) noexcept
  {
    auto blk = ref.alloc(sizeof(T));
    if constexpr (std::is_nothrow_constructible_v<T, Args...>)
    {
      new(blk.ptr()) T(std::forward<Args>(args)...);
    }
    else
    {
      try
      {
        new(blk.ptr()) T(std::forward<Args>(args)...);
      }
      catch (...)
      {
        ref.dealloc(blk);
        throw;
      }
    }
    
    return UniquePtr<T, mem::LocalAllocator<Alloc>>(ref, blk);
  }

  template<typename T, auto ALLOCATOR>
  /// @brief clt::hash overload for UniquePtr
  struct hash<UniquePtr<T, ALLOCATOR>>
  {
    /// @brief Hashing operator
    /// @param value The value to hash
    /// @return Hash
    constexpr size_t operator()(const UniquePtr<T, ALLOCATOR>& value) const noexcept
    {
      return hash_value(value.get());
    }
  };
}

#endif //!HG_COLT_UNIQUE_PTR