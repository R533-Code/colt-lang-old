/*****************************************************************/ /**
 * @file   block.h
 * @brief  Contains MemBlock, the result of an allocation.
 *
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_BLOCK
#define HG_COLT_BLOCK

#include "sizes.h"
#include "common/hash.h"

namespace clt::mem
{
  /// @brief Result of an allocation: ptr + size.
  /// If ptr() is nullptr, then size() is guaranteed to be 0.
  /// If size() is 0, ptr() is not guaranteed to be nullptr.
  class MemBlock
  {
    /// @brief Address of the block
    void* blk_ptr = nullptr;
    /// @brief Size of the block (in bytes)
    u64 blk_sz = 0;

  public:
    constexpr MemBlock() = default;
    /// @brief Constructs a MemBlock from a pointer and a size
    /// @param blk The block
    /// @param sz The size of the block
    constexpr MemBlock(void* blk, u64 sz = 0ULL) noexcept
        : blk_ptr(blk)
        , blk_sz(sz * static_cast<u64>(blk != nullptr))
    {
    }

    /// @brief Constructs a MemBlock from two pointers
    /// @param start The beginning of the block
    /// @param end The end of the block
    constexpr MemBlock(void* start, void* end) noexcept
        : blk_ptr(start)
        , blk_sz(static_cast<u8*>(end) - static_cast<u8*>(start))
    {
      assert_true("'start' must precede 'end'!", start < end);
      blk_sz *= static_cast<u64>(start != nullptr);
    }

    /// @brief Constructs a MemBlock from a nullptr.
    /// The size of a MemBlock with ptr() == nullptr is always 0.
    constexpr MemBlock(std::nullptr_t, u64 = 0) noexcept {}

    //Copy constructor
    constexpr MemBlock(const MemBlock&) noexcept = default;
    //Move constructor
    constexpr MemBlock(MemBlock&&) noexcept = default;
    //Copy-assignment operator
    constexpr MemBlock& operator=(const MemBlock&) noexcept = default;
    //Move-assignment operator
    constexpr MemBlock& operator=(MemBlock&&) noexcept = default;

    /// @brief Sets the block to nullptr
    /// @param  nullptr
    /// @return Self
    constexpr MemBlock& operator=(std::nullptr_t) noexcept
    {
      blk_ptr = nullptr;
      blk_sz  = 0;
      return *this;
    }

    /// @brief Returns the size of a block (in bytes).
    /// The size of a block cannot be modified directly, except by assignment.
    /// @return Byte size of the block
    constexpr ByteSize<Byte> size() const noexcept { return blk_sz; }
    /// @brief Returns the pointer to the memory block
    /// @return Pointer (can be nullptr)
    constexpr void* ptr() const noexcept { return blk_ptr; }

    /// @brief Check if 'ptr() == nullptr'
    /// @return True if the MemBlock points to no block
    constexpr bool is_null() const noexcept { return blk_ptr == nullptr; }
    /// @brief Check if 'ptr() != nullptr'
    /// @return True if the MemBlock points to a block
    explicit constexpr operator bool() const noexcept { return blk_ptr != nullptr; }

    /// @brief Check if two blocks are equal.
    /// Comparison of the pointer, then the sizes are done.
    /// @param blk The block to compare with
    /// @return True if both the block and size are equal
    constexpr bool operator==(const MemBlock&) const noexcept = default;

    /// @brief Check if a block is nullptr
    /// @param  nullptr
    /// @return True if nullptr
    constexpr bool operator==(std::nullptr_t) const noexcept
    {
      return blk_ptr == nullptr;
    }

    /// @brief Check if a block is not nullptr
    /// @param  nullptr
    /// @return True if not nullptr
    constexpr bool operator!=(std::nullptr_t) const noexcept
    {
      return blk_ptr != nullptr;
    }
  };

  template<typename T>
    requires(!std::is_array_v<T>)
  class TypedBlock
  {
    /// @brief Pointer to the block
    T* blk_ptr = nullptr;
    /// @brief Number of objects in the block
    size_t blk_count = 0;

  public:
    constexpr TypedBlock() noexcept = default;

    /// @brief Constructs a TypedBlock over a pointer and a size
    /// @param ptr The pointer to the memory block
    /// @param size The size of the memory block pointed by 'ptr'
    constexpr TypedBlock(void* ptr, size_t size) noexcept
        : blk_ptr(static_cast<T*>(ptr))
        , blk_count((size * static_cast<u64>(ptr != nullptr)) / sizeof(T))
    {
    }

    /// @brief Constructs a TypedBlock over a MemBlock
    /// @param blk The MemBlock
    constexpr TypedBlock(MemBlock blk) noexcept
        : blk_ptr(static_cast<T*>(blk.ptr()))
        , blk_count(blk.size().to_bytes() / sizeof(T))
    {
    }

    /// @brief Check if the block is empty (get_ptr() == nullptr)
    /// @return True if the block is empty
    constexpr bool is_empty() const noexcept { return blk_ptr == nullptr; }

    /// @brief Implicitly converts a block to a boolean, like a pointer
    /// @return True if the block is not empty
    constexpr explicit operator bool() const noexcept { return blk_ptr != nullptr; }
    /// @brief Implicitly converts a block to a boolean, like a pointer
    /// @return True if the block is empty
    constexpr bool operator!() const noexcept { return blk_ptr == nullptr; }

    /// @brief Dereferences the pointer to the memory block
    /// @return Const reference to the type of the block
    constexpr const T& operator*() const noexcept { return *blk_ptr; }
    /// @brief Dereferences the pointer to the memory block
    /// @return Reference to the type of the block
    constexpr T& operator*() noexcept { return *blk_ptr; }

    /// @brief Dereferences the pointer to the memory block
    /// @return Const reference to the type of the block
    constexpr const T& operator->() const noexcept { return *blk_ptr; }
    /// @brief Dereferences the pointer to the memory block
    /// @return Reference to the type of the block
    constexpr T& operator->() noexcept { return *blk_ptr; }

    /// @brief Get the pointer to the block
    /// @return Const pointer to the type of the block
    constexpr const T* ptr() const noexcept { return blk_ptr; }
    /// @brief Get the pointer to the block
    /// @return Pointer to the type of the block
    constexpr T* ptr() noexcept { return blk_ptr; }

    /// @brief Returns the count of objects in the block
    /// @return The count of objects
    constexpr size_t size() const noexcept { return blk_count; }

    /// @brief Convert a TypedBlock to a MemBlock
    /// @return MemBlock
    constexpr operator MemBlock() const noexcept
    {
      return MemBlock{static_cast<void*>(blk_ptr), blk_count * sizeof(T)};
    }
  };

  /// @brief Represents an empty block
  inline constexpr MemBlock nullblk = MemBlock{nullptr, 0};

  template<u64 ALIGN>
  /// @brief Rounds 'sz' to an alignment if it is not already aligned
  /// @param sz The size to align
  /// @return The aligned size
  constexpr size_t round_to_alignment(size_t sz) noexcept
  {
    //Do no round as already rounded
    if (sz % ALIGN == 0)
      return sz;
    //Round size upward if needed
    return sz + ALIGN - (sz % ALIGN);
  }

  namespace details
  {
    template<typename Old, typename New>
    /// @brief Reallocates using new allocator, copying the memory,
    /// and deallocating the block on success.
    /// @tparam New The type of the new allocator
    /// @tparam Old The type of the old allocator
    /// @param old_a The old allocator to use for deallocation
    /// @param new_a The new allocator to use for allocation
    /// @param blk The block to "reallocate"
    /// @param n The new size
    /// @return True on success
    constexpr bool realloc_with_copy(
        Old& old_a, New& new_a, MemBlock& blk, ByteSize<Byte> n) noexcept
    {
      MemBlock new_blk = new_a.alloc(n);
      if (new_blk.is_null())
      {
        return false;
      }
      std::memcpy(
          new_blk.ptr(), blk.ptr(),
          blk.size() < new_blk.size() ? blk.size().to_bytes()
                                      : new_blk.size().to_bytes());
      old_a.dealloc(blk);
      blk = new_blk;
      return true;
    }
  } // namespace details
} // namespace clt::mem

namespace clt
{
  template<>
  /// @brief clt::hash overload for MemBlock
  struct hash<mem::MemBlock>
  {
    /// @brief Hashing operator
    /// @param pair The value to hash
    /// @return Hash
    constexpr size_t operator()(const clt::mem::MemBlock& pair) const noexcept
    {
      size_t seed = 0xCBF29CE484222325;
      seed        = hash_combine(seed, hash_value(pair.ptr()));
      seed        = hash_combine(seed, hash_value(pair.size().to_bytes()));
      return seed;
    }
  };

  template<typename T>
  /// @brief clt::hash overload for MemBlock
  struct hash<mem::TypedBlock<T>>
  {
    /// @brief Hashing operator
    /// @param pair The value to hash
    /// @return Hash
    constexpr size_t operator()(const clt::mem::TypedBlock<T>& pair) const noexcept
    {
      size_t seed = 0xCBF29CE484222325;
      seed        = hash_combine(seed, hash_value(pair.ptr()));
      seed        = hash_combine(seed, hash_value(pair.size().to_bytes()));
      return seed;
    }
  };
} // namespace clt

template<>
struct fmt::formatter<clt::mem::MemBlock> : public clt::meta::DefaultParserFMT
{
  template<typename FormatContext>
  auto format(const clt::mem::MemBlock& vec, FormatContext& ctx)
  {
    return fmt::format_to(ctx.out(), "{{ {}, {} }}", vec.ptr(), vec.size());
  }
};

template<typename T>
struct fmt::formatter<clt::mem::TypedBlock<T>> : public clt::meta::DefaultParserFMT
{
  template<typename FormatContext>
  auto format(const clt::mem::TypedBlock<T>& vec, FormatContext& ctx)
  {
    return fmt::format_to(
        ctx.out(), "{{ {}, {} }}", static_cast<void*>(vec.ptr()), vec.size());
  }
};

#endif //!HG_COLT_BLOCK