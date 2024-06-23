/*****************************************************************/ /**
 * @file   simple_alloc.h
 * @brief  Contains simple allocators, building blocks for more advanced ones.
 *
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_SIMPLE_ALLOC
#define HG_COLT_SIMPLE_ALLOC

#include <cstdlib>
#include "allocator_traits.h"

namespace clt::mem
{
  /// @brief NULL allocator, returns and takes 'nullblk'
  class NULLAllocator
  {
  public:
    /// @brief Alignment of returned MemBlock (arbitrary)
    static constexpr u64 alignment = 1024;

    /// @brief Always returns a 'nullblk'
    /// @param  The size to allocate
    /// @return nullblk
    constexpr MemBlock alloc(ByteSize<Byte>) const noexcept { return nullblk; }

    /// @brief Asserts that 'blk' is a 'nullblk'
    /// @param blk The block to deallocate
    constexpr void dealloc(MemBlock blk) const noexcept
    {
      //Do nothing... we only verify that the block is null
      assert_true("NULLAllocator must only receive empty blocks!", blk == nullblk);
    }

    /// @brief Check if the block is owned by this allocator
    /// @param blk The block to check for
    /// @return True if the block is null
    constexpr bool owns(MemBlock blk) const noexcept { return blk.is_null(); }

    /// @brief Returns false
    /// @param blk The block to check for
    /// @param  Size for expansion
    /// @return false
    constexpr bool expand(MemBlock& blk, ByteSize<Byte>) const noexcept
    {
      assert_true("NULLAllocator must only receive empty blocks!", blk == nullblk);
      return false;
    }

    /// @brief Returns false
    /// @param blk The block to check for
    /// @param  Size for expansion
    /// @return false
    constexpr bool realloc(MemBlock& blk, ByteSize<Byte>) const noexcept
    {
      assert_true("NULLAllocator must only receive empty blocks!", blk == nullblk);
      return false;
    }
  };

  /// @brief Allocator that uses malloc/free
  class Mallocator
  {
  public:
    /// @brief Alignment of returned MemBlock
    static constexpr u64 alignment = __STDCPP_DEFAULT_NEW_ALIGNMENT__;

    /// @brief Allocates a block using 'malloc'
    /// @param size The size to allocate
    /// @return nullblk
    MemBlock alloc(ByteSize<Byte> size) const noexcept
    {
      assert_true("Mallocator must not allocate block of size '0'!", size != 0_B);
      return {std::malloc(size.to_bytes()), size.to_bytes()};
    }

    /// @brief Deallocates a block
    /// @param blk The block to deallocate
    void dealloc(MemBlock blk) const noexcept { std::free(blk.ptr()); }

    /// @brief Reallocates a MemBlock
    /// @param blk The block to reallocate
    /// @param n The new size of the block
    /// @return True if reallocation was successful
    bool realloc(MemBlock& blk, ByteSize<Byte> sz) const noexcept
    {
      auto ptr = std::realloc(blk.ptr(), sz.to_bytes());
      if (ptr == nullptr)
        return false;
      blk = MemBlock{ptr, sz.to_bytes()};
      return true;
    }
  };

  template<ByteSize<Byte> SIZE, u64 ALIGN = alignof(std::max_align_t)>
  /// @brief Allocator that uses stack memory to allocate
  class StackAllocator
  {
    /// @brief The buffer from which to return MemBlocks
    alignas(ALIGN) u8 buffer[SIZE.to_bytes()];
    /// @brief The top of the buffer
    u8* top = buffer;

    /// @brief Check if the stack has enough capacity to allocate 'sz'
    /// @param sz The aligned size to check
    /// @return True if the stack has enough capacity
    constexpr bool can_allocate(size_t sz) noexcept
    {
      return top + sz <= buffer + SIZE.to_bytes();
    }

    /// @brief Check if the stack is empty
    /// @return True if empty
    constexpr bool is_empty() noexcept { return top == buffer; }

  public:
    constexpr StackAllocator() noexcept   = default;
    StackAllocator(const StackAllocator&) = delete;
    StackAllocator(StackAllocator&&)      = delete;

    /// @brief Alignment of returned MemBlock
    static constexpr u64 alignment = ALIGN;

    /// @brief Check if the current allocator owns 'blk'
    /// @param blk The MemBlock to check
    /// @return True if 'blk' was allocated through the current allocator
    constexpr bool owns(MemBlock blk) const noexcept
    {
      return blk == nullblk || (buffer <= blk.ptr() && blk.ptr() < top);
    }

    /// @brief Allocates a MemBlock
    /// @param sze The size of the allocation
    /// @return Allocated MemBlock or an empty MemBlock on failure
    constexpr MemBlock alloc(ByteSize<Byte> sze) noexcept
    {
      size_t aligned_size = round_to_alignment<ALIGN>(sze.to_bytes());
      if (!can_allocate(aligned_size))
        return nullblk;
      auto ptr = top;
      top += aligned_size;
      return {ptr, sze.to_bytes()};
    }

    /// @brief Deallocates a MemBlock that was allocated using the current allocator
    /// @param to_free The block whose resources to free
    constexpr void dealloc(MemBlock to_free) noexcept
    {
      assert_true("StackAllocator must own the block to free!", this->owns(to_free));
      assert_true(
          "StackAllocator cannot free a block if it did not event allocate one!",
          to_free == nullblk || !this->is_empty());

      //This also works for 'nullblk'
      size_t aligned_size = round_to_alignment<ALIGN>(to_free.size().to_bytes());
      if (static_cast<u8*>(to_free.ptr()) + aligned_size == top)
        top -= aligned_size;
    }

    /// @brief Reallocates a MemBlock
    /// @param blk The block to reallocate
    /// @param n The new size of the block
    /// @return True if reallocation was successful
    constexpr bool realloc(MemBlock& blk, ByteSize<Byte> n) noexcept
    {
      assert_true("StackAllocator must own the block to realloc!", this->owns(blk));
      if (blk.size() == n)
        return true;
      if (blk.is_null())
      {
        blk = alloc(n);
        return !blk.is_null();
      }
      if (n == 0_B)
      {
        dealloc(blk);
        return true;
      }
      //If the block is the top one, and the stack
      //has enough space, "expand" the block in place
      if (static_cast<u8*>(blk.ptr()) + blk.size().to_bytes() == top)
      {
        if (can_allocate(n.to_bytes() - blk.size().to_bytes()))
        {
          top += round_to_alignment<ALIGN>(n.to_bytes() - blk.size().to_bytes());
          blk = MemBlock{blk.ptr(), n.to_bytes()};
          return true;
        }
        return false; //not enough memory
      }

      auto nblk = alloc(n);
      if (nblk.is_null())
        return false;
      std::memcpy(nblk.ptr(), blk.ptr(), blk.size().to_bytes());
      //We leak 'blk' as we cannot deallocate it.
      blk = nblk;
      return true;
    }

    /// @brief Expands a block in place if possible
    /// @param blk The block to expand
    /// @param delta The new size
    /// @return True if expansion was done successfully
    constexpr bool expand(MemBlock& blk, ByteSize<Byte> delta) noexcept
    {
      assert_true("StackAllocator must own the block to expand!", this->owns(blk));

      if (delta == 0_B)
        return true;

      if (blk.is_null())
      {
        blk = alloc(delta);
        return !blk.is_null();
      }
      if (static_cast<u8*>(blk.ptr()) + blk.size().to_bytes() != top)
        return false; //not the top

      if (can_allocate(delta.to_bytes() - blk.size().to_bytes()))
      {
        top += round_to_alignment<ALIGN>(delta.to_bytes() - blk.size().to_bytes());
        blk = MemBlock{blk.ptr(), delta.to_bytes()};
        return true;
      }
      return false; //not enough memory
    }
  };

  template<
      meta::Allocator allocator, ByteSize<Byte> LOWER, ByteSize<Byte> UPPER,
      size_t MAX_SAVED = 16>
    requires(LOWER >= 16_B) && (UPPER >= LOWER) && (MAX_SAVED != 0)
  /// @brief Allocator that reuses memory that is about to be freed.
  /// A FreeList is an amazing tool for allocations:
  /// When a block is about to be freed, the free list will check if
  /// it is in the range [UPPER, LOWER]. If the block is, the list
  /// will not deallocate the block, but store its size and address
  /// in a linked list. When allocating, if the size is in the range
  /// [UPPER, LOWER], the linked list will search in the linked list
  /// and return a saved block if its size matches.
  /// 'MAX_SAVED' fixes a limit of maximum items in the linked list.
  class FreeList : private allocator
  {
    /// @brief A Node contains a pointer to the next Node
    struct Node
    {
      u64 size;
      Node* next;
    };

    /// @brief Node to return on allocation
    Node* root = nullptr;
    /// @brief The count of saved items in the linked list
    u64 saved_count = 0;

    /// @brief Check if a size is in range
    /// @param n The size to check
    /// @return True if LOWER <= n && n <= UPPER
    constexpr bool is_in_range(size_t n) noexcept
    {
      return LOWER.to_bytes() <= n && n <= UPPER.to_bytes();
    }

    /// @brief Extracts a block from the linked list, of size 'size'
    /// @param size The size of the block to search for
    /// @return A block of size 'size' or nullptr if not found
    constexpr void* get_node_of_size(u64 size) noexcept
    {
      assert_true("Invalid state for FreeList", root != nullptr);
      auto pre = root;
      auto nxt = root->next;
      if (pre->size == size)
      {
        auto ptr = root;
        root     = root->next;
        return ptr;
      }
      while (nxt != nullptr)
      {
        if (nxt->size == size)
        {
          auto ptr  = nxt;
          pre->next = nxt->next;
          return ptr;
        }
        pre = nxt;
        nxt = nxt->next;
      }
      return nullptr;
    }

  public:
    constexpr FreeList() noexcept = default;
    FreeList(const FreeList&)     = delete;
    FreeList(FreeList&&)          = delete;

    /// @brief Alignment of returned MemBlock
    static constexpr u64 alignment = allocator::alignment;

    /// @brief Allocates a MemBlock
    /// @param size The size of the allocation
    /// @return Allocated MemBlock or an empty MemBlock on failure
    constexpr MemBlock alloc(ByteSize<Byte> n) noexcept
    {
      if (root != nullptr && is_in_range(n.to_bytes()))
      {
        if (auto find = get_node_of_size(n.to_bytes()); find)
          return {find, n.to_bytes()};
      }
      return allocator::alloc(n);
    }

    /// @brief Deallocates a MemBlock that was allocated using the current allocator
    /// @param to_free The block whose resources to free
    constexpr void dealloc(MemBlock blk) noexcept
    {
      //'nullblk' will never be registered: they are deallocated
      if (!is_in_range(blk.size().to_bytes()) || saved_count == MAX_SAVED)
      {
        allocator::dealloc(blk);
        return;
      }

      //Add to memory to the free list
      root = new (blk.ptr()) Node{blk.size().to_bytes(), root};
      //Add to save count
      ++saved_count;
    }

    /// @brief Check if the current allocator owns 'blk'
    /// @param blk The MemBlock to check
    /// @return True if 'blk' was allocated through the current allocator
    constexpr bool owns(MemBlock blk) const noexcept
      requires meta::OwningAllocator<allocator>
    {
      return allocator::owns(blk);
    }

    /// @brief Reallocates a MemBlock
    /// @param blk The block to reallocate
    /// @param n The new size of the block
    /// @return True if reallocation was successful
    constexpr bool realloc(MemBlock& blk, ByteSize<Byte> n) noexcept
      requires meta::ReallocatableAllocator<allocator>
    {
      return allocator::dealloc(blk, n);
    }

    /// @brief Expands a block in place if possible
    /// @param blk The block to expand
    /// @param delta The new size
    /// @return True if expansion was done successfully
    constexpr bool expand(MemBlock& blk, ByteSize<Byte> delta) noexcept
      requires meta::ExpandingAllocator<allocator>
    {
      return allocator::expand(blk);
    }

    /// @brief Returns all the block owned by the FreeList to the underlying allocator
    constexpr ~FreeList() noexcept
    {
      while (root != nullptr)
      {
        auto next = root->next;
        allocator::dealloc({static_cast<void*>(root), root->size});
        root = next;
      }
    }
  };
} // namespace clt::mem

#endif //!HG_COLT_SIMPLE_ALLOC