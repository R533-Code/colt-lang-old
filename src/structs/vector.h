/*****************************************************************//**
 * @file   vector.h
 * @brief  Contains a growable contiguous array.
 * This Vector class uses the new allocators provided in 'mem'.
 * To create a Vector that uses a local allocator, use make_local_vector.
 * 
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_VECTOR
#define HG_COLT_VECTOR

#include <utility>
#include <initializer_list>
#include <algorithm>
#include <span>

#include "mem/global_alloc.h"
#include "common.h"

namespace clt
{
  template<typename T>
  using View = std::span<const T, std::dynamic_extent>;

  template<typename T>
  using Span = std::span<T, std::dynamic_extent>;

  template<typename T, auto ALLOCATOR = mem::GlobalAllocatorDescription>
    requires meta::AllocatorScope<ALLOCATOR>
  /// @brief Dynamic size array, that can make use of a local allocator
  /// @tparam T The type stored in the Vector
  class Vector
  {
    /// @brief True if the allocator is global
    static constexpr bool is_global   = meta::GlobalAllocator<ALLOCATOR>;
    /// @brief True if the allocator is local (we need a reference to it)
    static constexpr bool is_local    = meta::LocalAllocator<ALLOCATOR>;

    /// @brief Type of the allocator
    using Allocator = mem::allocator_ref<ALLOCATOR>;

    /// @brief The allocator used for allocation/deallocation
    [[no_unique_address]]
    Allocator allocator;
    /// @brief Pointer to the allocated block (can be null)
    T* blk_ptr = nullptr;
    /// @brief Capacity (count) of objects of the block
    size_t blk_capacity = 0;
    /// @brief Count of active objects in the block
    size_t blk_size = 0;

    constexpr void reserve_obj(size_t plus_capacity)
      noexcept(std::is_nothrow_move_constructible_v<T>
        && std::is_nothrow_destructible_v<T>)
    {
      if (plus_capacity == 0)
        return;
      auto new_blk = allocator.alloc((blk_capacity + plus_capacity) * sizeof(T));
      //register freeing the memory to avoid leaks in case of exceptions
      ON_SCOPE_EXIT {
        if (blk_ptr)
          allocator.dealloc({ blk_ptr, blk_capacity * sizeof(T) });
        blk_ptr = static_cast<T*>(new_blk.ptr());
        blk_capacity = new_blk.size().to_bytes() / sizeof(T);
      };
      
      details::contiguous_destructive_move(blk_ptr, static_cast<T*>(new_blk.ptr()), blk_size);
    }

  public:
    /// @brief The value type stored in the list
    using value_type = T;

    /// @brief The maximum count of items that can be stored
    /// @return The maximum theoretical size of the container
    static size_t max_size() noexcept { return std::numeric_limits<size_t>::max(); }
    
    /// @brief Default constructor (when allocator is global)
    constexpr Vector() noexcept requires is_global = default;
    
    template<meta::Allocator AllocT> requires is_local
    /// @brief Default constructor (when allocator is local)
    /// @param alloc Reference to the allocator to use
    constexpr Vector(AllocT& alloc) noexcept
      : allocator(alloc) {}

    template<meta::Allocator AllocT> requires is_local
    /// @brief Reserve 'reserve' objects constructor (when allocator is local)
    /// @param alloc Reference to the allocator
    /// @param reserve The count of objects to allocate for
    constexpr Vector(AllocT& alloc, size_t reserve) noexcept
      : allocator(alloc)
    {
      reserve_obj(reserve);
    }

    /// @brief Default constructor (when allocator is global)
    /// @param reserve The count of objects to allocate for
    constexpr Vector(size_t reserve) noexcept requires is_global
    {
      reserve_obj(reserve);
    }

    template<meta::Allocator AllocT, size_t Extent> requires is_local
      /// @brief Reserve 'reserve' objects constructor (when allocator is local)
      /// @param alloc Reference to the allocator
      /// @param reserve The count of objects to allocate for
    constexpr Vector(AllocT& alloc, std::span<const T, Extent> to_copy) noexcept
      : Vector(alloc, to_copy.size())
    {
      details::contiguous_copy(to_copy.begin(), blk_ptr, to_copy.size());
      blk_size = to_copy.size();
    }

    /// @brief Default constructor (when allocator is global)
    /// @param reserve The count of objects to allocate for
    template<size_t Extent>
    constexpr Vector(std::span<const T, Extent> to_copy) noexcept requires is_global
      : Vector(to_copy.size())
    {
      details::contiguous_copy(to_copy.begin(), blk_ptr, to_copy.size());
      blk_size = to_copy.size();
    }

    template<meta::Allocator AllocT, typename... Args> requires is_local
    /// @brief Constructs 'size' objects using 'args'
    /// @tparam ...Args The parameter pack
    /// @param alloc Reference to the local allocator to use
    /// @param size The count of object to construct
    /// @param  Tag helper
    /// @param ...args The argument pack
    constexpr Vector(AllocT& alloc, size_t size, meta::InPlaceT, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
      : allocator(alloc)
    {
      reserve_obj(size);
      details::contiguous_construct(blk_ptr, size, std::forward<Args>(args)...);
      blk_size = size;
    }

    template<typename... Args> requires is_global
    /// @brief Constructs 'size' objects using 'args'
    /// @tparam ...Args The parameter pack
    /// @param size The count of object to construct
    /// @param  Tag helper
    /// @param ...args The argument pack
    constexpr Vector(size_t size, meta::InPlaceT, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
      reserve_obj(size);
      details::contiguous_construct(blk_ptr, size, std::forward<Args>(args)...);
      blk_size = size;
    }

    template<meta::Allocator AllocT> requires is_local
    /// @brief Constructs a Vector from an initializer_list
    /// @param alloc Reference to the local allocator to use
    /// @param list The initializer list
    constexpr Vector(AllocT& alloc, std::initializer_list<T> list)
      noexcept(std::is_nothrow_copy_constructible_v<T>)
      : allocator(alloc)
    {
      reserve_obj(std::size(list));
      blk_size = std::size(list);
      details::contiguous_copy(std::data(list), blk_ptr, blk_size);
    }

    /// @brief Constructs a Vector from an initializer_list
    /// @param list The initializer list
    constexpr Vector(std::initializer_list<T> list)
      noexcept(std::is_nothrow_copy_constructible_v<T>) requires is_global
    {
      reserve_obj(std::size(list));
      blk_size = std::size(list);
      details::contiguous_copy(std::data(list), blk_ptr, blk_size);
    }

    /// @brief Copy constructor, copy the content from 'to_copy'
    /// The capacity of the resulting Vector is the same as 'to_copy'.
    /// @param to_copy 
    /// @return 
    constexpr Vector(const Vector& to_copy)
      noexcept(std::is_nothrow_copy_constructible_v<T>)
      : allocator(to_copy.allocator)
    {
      reserve_obj(to_copy.blk_capacity);
      details::contiguous_copy(to_copy.blk_ptr, blk_ptr, to_copy.blk_size);
      blk_size = to_copy.blk_size;
    }
    
    /// @brief Destroy the active objects and copy the content from 'to_copy'.
    /// The capacity of the resulting Vector is the same as 'to_copy'.
    /// @param to_copy The Vector whose objects to copy
    /// @return Self
    constexpr Vector& operator=(const Vector& to_copy)
      noexcept(std::is_nothrow_copy_constructible_v<T>
        && std::is_nothrow_destructible_v<T>)
    {
      assert_true("Self assignment is prohibited!", &to_copy != this);
      details::contiguous_destruct(blk_ptr, blk_size);
      if (blk_capacity < to_copy.blk_capacity)
        reserve_obj(to_copy.blk_capacity - blk_capacity);
      details::contiguous_copy(to_copy.blk_ptr, blk_ptr, to_copy.blk_size);
      blk_size = to_copy.blk_size;
      return *this;
    }

    /// @brief Move constructor
    /// @param to_move The Vector whose resources to steal
    constexpr Vector(Vector&& to_move) noexcept
      : allocator(to_move.allocator)
      , blk_ptr(std::exchange(to_move.blk_ptr, nullptr))
      , blk_capacity(std::exchange(to_move.blk_capacity, 0))
      , blk_size(std::exchange(to_move.blk_size, 0)) {}

    /// @brief Move assignment operator, swaps every member (allocator included)
    /// @param to_move The Vector being assigned
    /// @return Self
    constexpr Vector& operator=(Vector&& to_move) noexcept
    {
      assert_true("Self assignment is prohibited!", &to_move != this);
      if constexpr (is_local)
        std::swap(to_move.allocator.ref, allocator.ref);
      std::swap(to_move.blk_ptr, blk_ptr);
      std::swap(to_move.blk_capacity, blk_capacity);
      std::swap(to_move.blk_size, blk_size);
      
      return *this;
    }

    /// @brief Destructor, destroy all active objects and free memory
    constexpr ~Vector()
      noexcept(std::is_nothrow_destructible_v<T>)
    {
      //register freeing even if destructor throws to avoid memory leaks
      ON_SCOPE_EXIT {
        if (blk_ptr)
          allocator.dealloc({ blk_ptr, blk_capacity * sizeof(T) });
        blk_size = 0;
      };
      details::contiguous_destruct(blk_ptr, blk_size);
    }

    /// @brief Returns a pointer to the beginning of the data
    /// @return Const pointer to the beginning of the data (can be null)
    constexpr const T* data() const noexcept { return blk_ptr; }
    /// @brief Returns a pointer to the beginning of the data
    /// @return Pointer to the beginning of the data (can be null)
    constexpr T* data() noexcept { return blk_ptr; }

    /// @brief Returns the count of active objects in the Vector
    /// @return The count of objects in the Vector
    constexpr size_t size() const noexcept { return blk_size; }
    /// @brief Returns the capacity of the current allocation
    /// @return The capacity of the current allocation
    constexpr size_t capacity() const noexcept { return blk_capacity; }

    /// @brief Returns the object at index 'index' of the Vector.
    /// @param index The index of the object
    /// @return The object at index 'index'
    constexpr const T& operator[](size_t index) const noexcept
    {
      assert_true("Invalid index!", index < this->size());
      return blk_ptr[index];
    }

    /// @brief Returns a reference to the object at index 'index' of the Vector.
    /// @param index The index of the object
    /// @return The object at index 'index'
    constexpr T& operator[](size_t index) noexcept
    {
      assert_true("Invalid index!", index < this->size());
      return blk_ptr[index];
    }

    /// @brief Check if the Vector does not contain any object.
    /// Same as: size() == 0
    /// @return True if the Vector is empty
    constexpr bool is_empty() const noexcept { return blk_size == 0; }

    /// @brief Reserve 'by_more' object
    /// @param by_more The count of object to reserve for
    constexpr void reserve(size_t by_more)
      noexcept(std::is_nothrow_move_constructible_v<T>
        && std::is_nothrow_destructible_v<T>)
    {
      reserve_obj(by_more);
    }

    /// @brief Push an object at the end of the Vector by copying
    /// @param to_copy The object to copy at the end of the Vector
    constexpr void push_back(const T& to_copy)
      noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
      if (blk_size == blk_capacity)
        reserve(blk_capacity + 16);
      new(blk_ptr + blk_size) T(to_copy);
      ++blk_size;
    }

    /// @brief Push an object at the end of the Vector by moving
    /// @tparam T_ SFINAE helper
    /// @tparam  SFINAE helper
    /// @param to_move The object to move at the end of the Vector
    constexpr void push_back(T&& to_move)
      noexcept(std::is_nothrow_move_constructible_v<T>) requires (!std::is_trivial_v<T>)
    {
      if (blk_size == blk_capacity)
        reserve(blk_capacity + 16);
      new(blk_ptr + blk_size) T(std::move(to_move));
      ++blk_size;
    }

    template<typename... Args>
    /// @brief Emplace an object at the end of the Vector
    /// @tparam ...Args The parameter pack
    /// @param  InPlaceT tag
    /// @param ...args The argument pack to forward to the constructor
    constexpr void push_back(meta::InPlaceT, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
      if (blk_size == blk_capacity)
        reserve(blk_capacity + 16);
      new(blk_ptr + blk_size) T(std::forward<Args>(args)...);
      ++blk_size;
    }

    /// @brief Pops an item from the back of the Vector.
    constexpr void pop_back()
      noexcept(std::is_nothrow_destructible_v<T>)
    {
      assert_true("Vector is empty!", !this->is_empty());
      --blk_size;
      blk_ptr[blk_size].~T();
    }

    /// @brief Pops N item from the back of the Vector.
    /// @param N The number of item to pop from the back
    constexpr void pop_back_n(size_t N)
      noexcept(std::is_nothrow_destructible_v<T>)
    {
      assert_true("Vector is does not contain enought elements!", N <= this->size());
      for (size_t i = blk_size - N; i < blk_size; i++)
        blk_ptr[i].~T();
      blk_size -= N;
    }

    /// @brief Removes all the item from the Vector.
    /// This does not modify the capacity of the Vector.
    constexpr void clear()
      noexcept(std::is_nothrow_destructible_v<T>)
    {
      details::contiguous_destruct(blk_ptr, blk_size);
      blk_size = 0;
    }

    /// @brief Returns the first item in the Vector.
    /// @return The first item in the Vector.
    constexpr const T& front() const noexcept
    {
      assert_true("Vector is empty!", !this->is_empty());
      return *blk_ptr;
    }
    
    /// @brief Returns the first item in the Vector.
    /// @return The first item in the Vector.
    constexpr T& front() noexcept
    {
      assert_true("Vector is empty!", !this->is_empty());
      return *blk_ptr;
    }

    /// @brief Returns the last item in the Vector.
    /// @return The last item in the Vector.
    constexpr const T& back() const noexcept
    {
      assert_true("Vector is empty!", !this->is_empty());
      return blk_ptr[blk_size - 1];
    }
    
    /// @brief Returns the last item in the Vector.
    /// @return The last item in the Vector.
    constexpr T& back() noexcept
    {
      assert_true("Vector is empty!", !this->is_empty());
      return blk_ptr[blk_size - 1];
    }

    /// @brief Returns an iterator the beginning of the Vector
    /// @return Iterator to the beginning of the Vector
    constexpr T* begin() noexcept { return blk_ptr; }
    /// @brief Returns an iterator the beginning of the Vector
    /// @return Iterator to the beginning of the Vector
    constexpr const T* begin() const noexcept { return blk_ptr; }

    /// @brief Returns an iterator the end of the Vector
    /// @return Iterator to the end of the Vector
    constexpr T* end() noexcept { return blk_ptr + blk_size; }
    /// @brief Returns an iterator the end of the Vector
    /// @return Iterator to the end of the Vector
    constexpr const T* end() const noexcept { return blk_ptr + blk_size; }

    /// @brief Unsafe, changes the size to 'size'
    /// @param size The new size
    constexpr void _Unsafe_size(size_t size) noexcept
    {
      blk_size = size;
    }    

    /// @brief Converts a Vector to a View
    /// @return View over the whole Vector
    constexpr operator View<T>() const noexcept
    {
      return { begin(), end() };
    }

    /// @brief Converts a Vector to a Span
    /// @return Span over the whole Vector
    constexpr operator Span<T>() noexcept
    {
      return { begin(), end() };
    }

    constexpr Span<T> to_view() noexcept { return *this; }
    constexpr View<T> to_view() const noexcept { return *this; }

    /// @brief Check if every object of v1 and v2 are equal
    /// @param v1 The first Vector
    /// @param v2 The second Vector
    /// @return True if both Vector are equal
    friend constexpr bool operator==(const Vector& v1, View<T> v2) noexcept
    {
      if (v1.size() != v2.size())
        return false;
      for (size_t i = 0; i < v1.size(); i++)
        if (v1[i] != v2[i])
          return false;
      return true;
    }

    /// @brief Lexicographically compare two vectors
    /// @param v1 The first vector
    /// @param v2 The second vector
    /// @return Result of comparison
    friend constexpr auto operator<=>(const Vector& v1, View<T> v2) noexcept
    {
      return std::lexicographical_compare_three_way(
        v1.begin(), v1.end(), v2.begin(), v2.end()
      );
    }
  };

  template<typename T, meta::Allocator Alloc, typename... Args>
  /// @brief Constructs a Vector using a local allocator
  constexpr Vector<T, mem::LocalAllocator<Alloc>> make_local_vector(Alloc& ref, Args&&... args) noexcept
  {
    return Vector<T, mem::LocalAllocator<Alloc>>{ref, std::forward<Args>(args)...};
  }
  
  template<typename T, auto ALLOCATOR> requires meta::is_hashable_v<T>
  /// @brief clt::hash overload for Vector
  struct hash<Vector<T, ALLOCATOR>>
  {
    /// @brief Hashing operator
    /// @param value The value to hash
    /// @return Hash
    constexpr size_t operator()(const Vector<T, ALLOCATOR>& value) const noexcept
    {      
      return hash_value(value.to_view());
    }
  };
}

template<typename T, auto ALLOCATOR> requires clt::meta::Parsable<T>
struct scn::scanner<clt::Vector<T, ALLOCATOR>>
  : scn::empty_parser
{
  template <typename Context>
  error scan(clt::Vector<T, ALLOCATOR>& val, Context& ctx)
  {
    auto r = scn::scan_list_ex(ctx.range(), val, scn::list_separator(','));
    ctx.range() = std::move(r.range());
    return r.error();
  }
};

template<typename T, auto ALLOCATOR> requires fmt::is_formattable<T>::value
struct fmt::formatter<clt::Vector<T, ALLOCATOR>>
{
  bool human_readable = false;

  template<typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    auto it = ctx.begin();
    auto end = ctx.end();
    if (it == end)
      return it;
    if (*it == 'h')
    {
      ++it;
      human_readable = true;
    }
    assert_true("Possible format for Vector are: {} or {:h}!", *it == '}');
    return it;
  }

  template<typename FormatContext>
  auto format(const clt::Vector<T, ALLOCATOR>& vec, FormatContext& ctx)
  {
    auto fmt_to = ctx.out();
    if (human_readable)
    {
      if (!vec.is_empty())
        fmt_to = fmt::format_to(fmt_to, "{}", vec.front());
      if (vec.size() > 1)
      {
        for (size_t i = 1; i < vec.size() - 1; i++)
          fmt_to = fmt::format_to(fmt_to, ", {}", vec[i]);
        fmt_to = fmt::format_to(fmt_to, " and {}", vec.back());
      }
      return fmt_to;
    }
    else
    {
      fmt_to = fmt::format_to(fmt_to, "[");
      if (!vec.is_empty())
        fmt_to = fmt::format_to(fmt_to, "{}", vec.front());
      for (size_t i = 1; i < vec.size(); i++)
        fmt_to = fmt::format_to(fmt_to, ", {}", vec[i]);
      return fmt::format_to(fmt_to, "]");
    }
  }
};

#endif //!HG_COLT_VECTOR