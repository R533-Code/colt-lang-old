/*****************************************************************//**
 * @file   set.h
 * @brief  Contains a StableSet which guarantees iterator validity
 *         through its lifetime.
 * 
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_SET
#define HG_COLT_SET

#include "vector.h"
#include "list.h"
#include "common/hash.h"

namespace clt
{
  template<typename T, size_t PER_NODE = 256, auto ALLOCATOR = mem::GlobalAllocatorDescription>
    requires meta::is_hashable_v<T> && meta::AllocatorScope<ALLOCATOR>
  /// @brief An ordered container without duplicates that guarantees iterator validity for its lifetime.
  /// This StableSet is implemented using an internal hash table and a doubly linked list.
  /// The doubly linked list is a 'FlatList', of 'PER_NODE' equal to 'PER_NODE', which
  /// preserves the insertion order, and iterator validity.
  /// @tparam T The type to store
  class StableSet
  {
    using Slot = std::pair<size_t, T*>;

    /// @brief True if the allocator is global
    static constexpr bool is_global = meta::GlobalAllocator<ALLOCATOR>;
    /// @brief True if the allocator is local (we need a reference to it)
    static constexpr bool is_local = meta::LocalAllocator<ALLOCATOR>;

    /// @brief Type of the allocator
    using Allocator = mem::allocator_ref<ALLOCATOR>;

    /// @brief The allocator used for allocation/deallocation
    [[no_unique_address]]
    Allocator allocator;

    /// @brief Contains meta-data information about the slots of the map
    Vector<details::KeySentinel> sentinel_metadata = {};
    /// @brief Pointer to the slots
    Slot* slots_ptr = nullptr;
    /// @brief Capacity of the slots
    size_t slots_capacity = 0;
    /// @brief The list containing the objects
    FlatList<T, PER_NODE> list = {};
    /// @brief The load factor before reallocation
    float load_factor = 0.70f;

  public:
    /// @brief The value type stored in the list
    using value_type = T;

    /// @brief The maximum count of items that can be stored
    /// @return The maximum theoretical size of the container
    static size_t max_size() noexcept { return std::numeric_limits<size_t>::max(); }

    /// @brief Constructor
    /// @param load_factor The load factor (> 0.0f && < 1.0f)
    constexpr StableSet(float load_factor = 0.70f) noexcept
      : sentinel_metadata(16, InPlace, details::EMPTY)
      , slots_ptr(static_cast<Slot*>(allocator.alloc(16 * sizeof(Slot)).ptr()))
      , slots_capacity(16)
      , list(16 / PER_NODE)
      , load_factor(load_factor)
    {
      assert_true("Invalid load factor!",
        0.0f < load_factor && load_factor < 1.0f);
    }

    /// @brief Constructor, which reserves 'reserve_size' capacity for objects
    /// @param reserve_size The capacity to reserve
    /// @param load_factor The load factor (> 0.0f && < 1.0f)
    constexpr StableSet(size_t reserve_size, float load_factor = 0.70f) noexcept
      : sentinel_metadata(reserve_size, InPlace, details::EMPTY)
      , slots_ptr(static_cast<Slot*>(allocator.alloc(reserve_size * sizeof(Slot))))
      , slots_capacity(reserve_size)
      , list(reserve_size / PER_NODE)
      , load_factor(load_factor)
    {
      assert_true("Invalid load factor!",
        0.0f < load_factor && load_factor < 1.0f);
    }

    constexpr StableSet(const StableSet&) = delete;

    /// @brief Move constructor
    /// @param set The set to move
    constexpr StableSet(StableSet&& set) noexcept
      : sentinel_metadata(std::move(set.sentinel_metadata))
      , slots_ptr(std::exchange(set.slots_ptr, nullptr))
      , slots_capacity(std::exchange(set.slots_capacity, 0))
      , list(std::move(set.list))
      , load_factor(set.load_factor)
    {}

    /// @brief Destructor of the StableSet
    ~StableSet()
      noexcept(std::is_nothrow_destructible_v<T>)
    {
      for (size_t i = 0; i < sentinel_metadata.size(); i++)
      {
        if (details::is_sentinel_active(sentinel_metadata[i]))
          slots_ptr[i].~Slot(); //destroy active slots
      }
      allocator.dealloc(mem::MemBlock{ slots_ptr, slots_capacity * sizeof(Slot) });
    }

    /// @brief Returns the number of active unique elements in the StableSet
    /// @return The count of active unique elements
    constexpr size_t size() const noexcept { return list.size(); }

    /// @brief Returns the capacity of the internal hash map used by the StableSet.
    /// @return The capacity of the internal map of the StableSet
    constexpr size_t capacity() const noexcept { return slots_capacity; }

    /// @brief Check if the StableSet is empty
    /// @return True if empty
    constexpr bool is_empty() const noexcept { return list.size() == 0; }

    /// @brief Returns const iterators to the beginning of the StableSet
    /// @return Iterator to the beginning of the set
    constexpr auto begin() const noexcept -> decltype(list.begin()) { return list.begin(); }
    /// @brief Returns const iterators to the end of the StableSet
    /// @return Iterator to the end of the set
    constexpr auto end() const noexcept -> decltype(list.begin()) { return list.end(); }

    /// @brief Return the nth value inserted (with n being index)
    /// @param index The insertion number
    /// @return The nth value inserted (with n being index)
    constexpr const T& operator[](size_t index) const noexcept
    {
      assert_true("Invalid index for StableSet", index < this->size());
      return list[index];
    }

    /// @brief Check if the internal hash map used by the StableSet will rehash on the next insertion
    /// @return True if the next insert will rehash the internal map of the StableSet
    constexpr bool will_reallocate() const noexcept
    {
      return float(size() + 1) > load_factor * capacity();
    }

    /// @brief Returns the load factor of the Map
    /// @return The load factor
    constexpr float get_load_factor() const noexcept { return load_factor; }
    /// @brief Sets the load factor to 'nload_factor'.
    /// Precondition: nload_factor < 1.0f && nload_factor > 0.0f.
    /// @param nload_factor The new load factor
    constexpr void set_load_factor(float nload_factor) noexcept
    {
      assert_true("Invalid load factor!", nload_factor < 1.0f && nload_factor > 0.0f);
      load_factor = nload_factor;
    }

    /// @brief Returns a const reference to the internal list used by the StableSet
    /// @return Const reference to the list
    constexpr const FlatList<T, PER_NODE>& internal_list() const noexcept { return list; }

    /// @brief Inserts a new value if it does not already exist.
    /// Returns an InsertionResult SUCCESS (if the insertion was performed) or EXISTS (if the key already exists).
    /// The returned pointer is to the newly inserted value on SUCCESS.
    /// The returned pointer is to the existing value on EXISTS.
    /// The returned pointer is never null.
    /// @param key The value to insert
    /// @return Pair of pointer to the inserted slot or the existent one, and SUCESS on insertion or EXISTS if the key already exists
    constexpr std::pair<T*, InsertionResult> insert(const T& key)
      noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
      if (will_reallocate())
        realloc_map(capacity() + 16);

      const size_t key_hash = hash_value(key);
      size_t prob_index;
      if (find_key(key_hash, key, prob_index, sentinel_metadata, slots_ptr, slots_capacity))
      {
        list.push_back(key);
        T* to_ret = &list[list.size() - 1]; // always safe as push_backed the value
        new(slots_ptr + prob_index) Slot(key_hash, to_ret);
        //Set the slot to ACTIVE
        sentinel_metadata[prob_index] = details::create_active_sentinel(key_hash);
        return { to_ret, InsertionResult::SUCCESS };
      }
      else
        return { slots_ptr[prob_index].second, InsertionResult::EXISTS };
    }

    /// @brief Inserts a new value if it does not already exist.
    /// Returns an InsertionResult SUCCESS (if the insertion was performed) or EXISTS (if the key already exists).
    /// The returned pointer is to the newly inserted value on SUCCESS.
    /// The returned pointer is to the existing value on EXISTS.
    /// The returned pointer is never null.
    /// @tparam T_ SFINAE helper
    /// @tparam  SFINAE helper
    /// @param key The value to insert
    /// @return Pair of pointer to the inserted slot or the existent one, and SUCESS on insertion or EXISTS if the key already exists
    constexpr std::pair<T*, InsertionResult> insert(T&& key)
      noexcept(std::is_nothrow_move_constructible_v<T>) requires (!std::is_trivial_v<T>)
    {
      if (will_reallocate())
        realloc_map(capacity() + 16);

      const size_t key_hash = hash_value(key);
      size_t prob_index;
      if (find_key(key_hash, key, prob_index, sentinel_metadata, slots_ptr, slots_capacity))
      {
        list.push_back(std::move(key));
        T* to_ret = &list[list.size() - 1]; // always safe as push_backed the value
        new(slots_ptr + prob_index) Slot(key_hash, to_ret);
        //Set the slot to ACTIVE
        sentinel_metadata[prob_index] = details::create_active_sentinel(key_hash);
        return { to_ret, InsertionResult::SUCCESS };
      }
      else
        return { slots_ptr[prob_index].second, InsertionResult::EXISTS };
    }

    template<typename U>
    /// @brief Inserts a value and discards the result of 'insert'
    /// @param val The value to insert
    constexpr void push_back(U&& val)
    {
      insert(std::forward<U>(val));
    }

  private:
    /// @brief Finds a EMPTY/ACTIVE/DELETED slot matching 'key_hash'
    /// @param key_hash The hash of 'key', obtained through 'hash_value'.
    /// This function does not perform a hash of 'key' as usually the function
    /// that calls this function already possesses that hash.
    /// @param key The key to search for.
    /// This key will not be hashed by the function.
    /// @param prob The reference where to write the offset to the slot
    /// @param metadata The Vector of KeySentinel representing the state of 'blk'
    /// @param blk The array of slots
    /// @return True if the slot found is empty/deleted, false if the slot is already occupied
    static constexpr bool find_key(size_t key_hash, const T& key, size_t& prob,
      const Vector<details::KeySentinel>& metadata, Slot* ptr, size_t capacity) noexcept
    {
      assert_true("Ensure hash match!", key_hash == hash_value(key));
      assert_true("", metadata.size() == capacity);
      size_t prob_index = key_hash % capacity;
      for (;;)
      {
        if (auto sentinel = metadata[prob_index];
          details::is_sentinel_empty(sentinel) || details::is_sentinel_deleted(sentinel))
        {
          prob = prob_index;
          return true;
        }
        else if (details::is_sentinel_equal(sentinel, key_hash)
          && *(ptr[prob_index].second) == key)
        {
          prob = prob_index;
          return false;
        }
        prob_index = details::advance_prob(prob_index, capacity);
      }
    }

    /// @brief Augments the capacity of the internal hash map, rehashing in the process
    /// @param new_capacity The new capacity of the map
    constexpr void realloc_map(size_t new_capacity) noexcept
    {
      auto new_slot = static_cast<Slot*>(allocator.alloc(new_capacity * sizeof(Slot)).ptr());

      Vector<details::KeySentinel> new_metadata = { new_capacity, InPlace, details::EMPTY };
      for (size_t i = 0; i < sentinel_metadata.size(); i++)
      {
        auto sentinel = sentinel_metadata[i];
        if (details::is_sentinel_active(sentinel))
        {
          //find the key
          Slot* ptr = slots_ptr + i;
          size_t prob_index;
          //Rehash the key to get its new index in the new array
          if (find_key(ptr->first, *ptr->second, prob_index, new_metadata, new_slot, new_capacity))
          {
            //Set the slot to ACTIVE
            new_metadata[prob_index] = details::create_active_sentinel(ptr->first);

            //Move destruct
            new(new_slot + prob_index) Slot(std::move(*ptr));
            ptr->~Slot();
          }
        }
      }
      sentinel_metadata = std::move(new_metadata);
      allocator.dealloc(mem::MemBlock{ slots_ptr, slots_capacity * sizeof(Slot) });
      slots_ptr = new_slot;
      slots_capacity = new_capacity;
    }
  };

  template<typename T, auto ALLOCATOR = mem::GlobalAllocatorDescription>
    requires meta::is_hashable_v<T>&& meta::AllocatorScope<ALLOCATOR>
  /// @brief An ordered container without duplicates that guarantees index validity in between adds.
  /// This IndexedSet is implemented using an internal hash table and a Vector.
  /// @tparam T The type to store
  class IndexedSet
  {
    using Slot = std::pair<size_t, size_t>;

    /// @brief True if the allocator is global
    static constexpr bool is_global = meta::GlobalAllocator<ALLOCATOR>;
    /// @brief True if the allocator is local (we need a reference to it)
    static constexpr bool is_local = meta::LocalAllocator<ALLOCATOR>;

    /// @brief Type of the allocator
    using Allocator = mem::allocator_ref<ALLOCATOR>;

    /// @brief The allocator used for allocation/deallocation
    [[no_unique_address]]
    Allocator allocator;

    /// @brief Contains meta-data information about the slots of the map
    Vector<details::KeySentinel> sentinel_metadata = {};
    /// @brief Pointer to the slots
    Slot* slots_ptr = nullptr;
    /// @brief Capacity of the slots
    size_t slots_capacity = 0;
    /// @brief The list containing the objects
    Vector<T> list = {};
    /// @brief The load factor before reallocation
    float load_factor = 0.70f;

  public:
    /// @brief The value type stored in the list
    using value_type = T;

    /// @brief The maximum count of items that can be stored
    /// @return The maximum theoretical size of the container
    static size_t max_size() noexcept { return std::numeric_limits<size_t>::max(); }

    /// @brief Constructor
    /// @param load_factor The load factor (> 0.0f && < 1.0f)
    constexpr IndexedSet(float load_factor = 0.70f) noexcept
      : sentinel_metadata(16, InPlace, details::EMPTY)
      , slots_ptr(static_cast<Slot*>(allocator.alloc(16 * sizeof(Slot)).ptr()))
      , slots_capacity(16)
      , list(16)
      , load_factor(load_factor)
    {
      assert_true("Invalid load factor!",
        0.0f < load_factor && load_factor < 1.0f);
    }

    /// @brief Constructor, which reserves 'reserve_size' capacity for objects
    /// @param reserve_size The capacity to reserve
    /// @param load_factor The load factor (> 0.0f && < 1.0f)
    constexpr IndexedSet(size_t reserve_size, float load_factor = 0.70f) noexcept
      : sentinel_metadata(reserve_size, InPlace, details::EMPTY)
      , slots_ptr(static_cast<Slot*>(allocator.alloc(reserve_size * sizeof(Slot))))
      , slots_capacity(reserve_size)
      , list(reserve_size)
      , load_factor(load_factor)
    {
      assert_true("Invalid load factor!",
        0.0f < load_factor && load_factor < 1.0f);
    }

    constexpr IndexedSet(const IndexedSet&) = delete;

    /// @brief Move constructor
    /// @param set The set to move
    constexpr IndexedSet(IndexedSet&& set) noexcept
      : sentinel_metadata(std::move(set.sentinel_metadata))
      , slots_ptr(std::exchange(set.slots_ptr, nullptr))
      , slots_capacity(std::exchange(set.slots_capacity, 0))
      , list(std::move(set.list))
      , load_factor(set.load_factor)
    {}

    /// @brief Destructor of the StableSet
    ~IndexedSet()
      noexcept(std::is_nothrow_destructible_v<T>)
    {
      allocator.dealloc(mem::MemBlock{ slots_ptr, slots_capacity * sizeof(Slot) });
    }

    /// @brief Returns the number of active unique elements in the StableSet
    /// @return The count of active unique elements
    constexpr size_t size() const noexcept { return list.size(); }

    /// @brief Returns the capacity of the internal hash map used by the StableSet.
    /// @return The capacity of the internal map of the StableSet
    constexpr size_t capacity() const noexcept { return slots_capacity; }

    /// @brief Check if the StableSet is empty
    /// @return True if empty
    constexpr bool is_empty() const noexcept { return list.size() == 0; }

    /// @brief Returns const iterators to the beginning of the StableSet
    /// @return Iterator to the beginning of the set
    constexpr auto begin() const noexcept -> decltype(list.begin()) { return list.begin(); }
    /// @brief Returns const iterators to the end of the StableSet
    /// @return Iterator to the end of the set
    constexpr auto end() const noexcept -> decltype(list.begin()) { return list.end(); }

    /// @brief Return the nth value inserted (with n being index)
    /// @param index The insertion number
    /// @return The nth value inserted (with n being index)
    constexpr const T& operator[](size_t index) const noexcept
    {
      assert_true("Invalid index for IndexedSet", index < this->size());
      return list[index];
    }

    /// @brief Check if the internal hash map used by the StableSet will rehash on the next insertion
    /// @return True if the next insert will rehash the internal map of the StableSet
    constexpr bool will_reallocate() const noexcept
    {
      return float(size() + 1) > load_factor * capacity();
    }

    /// @brief Returns the load factor of the Map
    /// @return The load factor
    constexpr float get_load_factor() const noexcept { return load_factor; }
    /// @brief Sets the load factor to 'nload_factor'.
    /// Precondition: nload_factor < 1.0f && nload_factor > 0.0f.
    /// @param nload_factor The new load factor
    constexpr void set_load_factor(float nload_factor) noexcept
    {
      assert_true("Invalid load factor!", nload_factor < 1.0f && nload_factor > 0.0f);
      load_factor = nload_factor;
    }

    /// @brief Returns a const reference to the internal list used by the StableSet
    /// @return Const reference to the list
    constexpr const Vector<T>& internal_list() const noexcept { return list; }

    /// @brief Inserts a new value if it does not already exist.
    /// Returns an InsertionResult SUCCESS (if the insertion was performed) or EXISTS (if the key already exists).
    /// The returned index is to the newly inserted value on SUCCESS.
    /// The returned index is to the existing value on EXISTS.
    /// The returned index is always valid.
    /// @param key The value to insert
    /// @return Pair of pointer to the inserted slot or the existent one, and SUCESS on insertion or EXISTS if the key already exists
    constexpr std::pair<u64, InsertionResult> insert(const T& key)
      noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
      if (will_reallocate())
        realloc_map(capacity() + 16);

      const size_t key_hash = hash_value(key);
      size_t prob_index;
      if (find_key(key_hash, key, prob_index, sentinel_metadata, list, slots_ptr, slots_capacity))
      {
        size_t to_ret = list.size();
        list.push_back(key);
        new(slots_ptr + prob_index) Slot(key_hash, to_ret);
        //Set the slot to ACTIVE
        sentinel_metadata[prob_index] = details::create_active_sentinel(key_hash);
        return { to_ret, InsertionResult::SUCCESS };
      }
      else
        return { slots_ptr[prob_index].second, InsertionResult::EXISTS };
    }

    /// @brief Inserts a new value if it does not already exist.
    /// Returns an InsertionResult SUCCESS (if the insertion was performed) or EXISTS (if the key already exists).
    /// The returned pointer is to the newly inserted value on SUCCESS.
    /// The returned pointer is to the existing value on EXISTS.
    /// The returned pointer is never null.
    /// @tparam T_ SFINAE helper
    /// @tparam  SFINAE helper
    /// @param key The value to insert
    /// @return Pair of pointer to the inserted slot or the existent one, and SUCESS on insertion or EXISTS if the key already exists
    constexpr std::pair<u64, InsertionResult> insert(T&& key)
      noexcept(std::is_nothrow_move_constructible_v<T>) requires (!std::is_trivial_v<T>)
    {
      if (will_reallocate())
        realloc_map(capacity() + 16);

      const size_t key_hash = hash_value(key);
      size_t prob_index;
      if (find_key(key_hash, key, prob_index, sentinel_metadata, list, slots_ptr, slots_capacity))
      {
        size_t to_ret = list.size();
        list.push_back(std::move(key));
        new(slots_ptr + prob_index) Slot(key_hash, to_ret);
        //Set the slot to ACTIVE
        sentinel_metadata[prob_index] = details::create_active_sentinel(key_hash);
        return { to_ret, InsertionResult::SUCCESS };
      }
      else
        return { slots_ptr[prob_index].second, InsertionResult::EXISTS };
    }

    template<typename U>
    /// @brief Inserts a value and discards the result of 'insert'
    /// @param val The value to insert
    constexpr void push_back(U&& val)
    {
      insert(std::forward<U>(val));
    }

  private:
    /// @brief Finds a EMPTY/ACTIVE/DELETED slot matching 'key_hash'
    /// @param key_hash The hash of 'key', obtained through 'hash_value'.
    /// This function does not perform a hash of 'key' as usually the function
    /// that calls this function already possesses that hash.
    /// @param key The key to search for.
    /// This key will not be hashed by the function.
    /// @param prob The reference where to write the offset to the slot
    /// @param metadata The Vector of KeySentinel representing the state of 'blk'
    /// @param blk The array of slots
    /// @return True if the slot found is empty/deleted, false if the slot is already occupied
    static constexpr bool find_key(size_t key_hash, const T& key, size_t& prob,
      const Vector<details::KeySentinel>& metadata, const Vector<T>& list, Slot* ptr, size_t capacity) noexcept
    {
      assert_true("Ensure hash match!", key_hash == hash_value(key));
      assert_true("", metadata.size() == capacity);
      size_t prob_index = key_hash % capacity;
      for (;;)
      {
        if (auto sentinel = metadata[prob_index];
          details::is_sentinel_empty(sentinel) || details::is_sentinel_deleted(sentinel))
        {
          prob = prob_index;
          return true;
        }
        else if (details::is_sentinel_equal(sentinel, key_hash)
          && list[ptr[prob_index].second] == key)
        {
          prob = prob_index;
          return false;
        }
        prob_index = details::advance_prob(prob_index, capacity);
      }
    }

    /// @brief Augments the capacity of the internal hash map, rehashing in the process
    /// @param new_capacity The new capacity of the map
    constexpr void realloc_map(size_t new_capacity) noexcept
    {
      auto new_slot = static_cast<Slot*>(allocator.alloc(new_capacity * sizeof(Slot)).ptr());

      Vector<details::KeySentinel> new_metadata = { new_capacity, InPlace, details::EMPTY };
      for (size_t i = 0; i < sentinel_metadata.size(); i++)
      {
        auto sentinel = sentinel_metadata[i];
        if (details::is_sentinel_active(sentinel))
        {
          //find the key
          Slot* ptr = slots_ptr + i;
          size_t prob_index;
          //Rehash the key to get its new index in the new array
          if (find_key(ptr->first, list[ptr->second], prob_index, new_metadata, list, new_slot, new_capacity))
          {
            //Set the slot to ACTIVE
            new_metadata[prob_index] = details::create_active_sentinel(ptr->first);

            //Move destruct
            new(new_slot + prob_index) Slot(std::move(*ptr));
          }
        }
      }
      sentinel_metadata = std::move(new_metadata);
      allocator.dealloc(mem::MemBlock{ slots_ptr, slots_capacity * sizeof(Slot) });
      slots_ptr = new_slot;
      slots_capacity = new_capacity;
    }
  };
}

template<typename T, size_t PER_NODE, auto ALLOCATOR>
  requires clt::meta::Parsable<T>
struct scn::scanner<clt::StableSet<T, PER_NODE, ALLOCATOR>>
  : scn::empty_parser
{
  template <typename Context>
  error scan(clt::StableSet<T, PER_NODE, ALLOCATOR>& val, Context& ctx)
  {
    auto r = scn::scan_list_ex(ctx.range(), val, scn::list_separator(','));
    ctx.range() = std::move(r.range());
    return r.error();
  }
};

template<typename T, auto ALLOCATOR>
  requires clt::meta::Parsable<T>
struct scn::scanner<clt::IndexedSet<T, ALLOCATOR>>
  : scn::empty_parser
{
  template <typename Context>
  error scan(clt::IndexedSet<T, ALLOCATOR>& val, Context& ctx)
  {
    auto r = scn::scan_list_ex(ctx.range(), val, scn::list_separator(','));
    ctx.range() = std::move(r.range());
    return r.error();
  }
};

template<typename T, auto ALLOCATOR>
  requires fmt::is_formattable<T>::value
struct fmt::formatter<clt::IndexedSet<T, ALLOCATOR>>
  : fmt::formatter<clt::Vector<T, ALLOCATOR>>
{
  template<typename FormatContext>
  auto format(const clt::IndexedSet<T, ALLOCATOR>& vec, FormatContext& ctx)
  {
    return fmt::formatter<clt::Vector<T, ALLOCATOR>>::format(vec.internal_list(), ctx);
  }
};

#endif //!HG_COLT_SET