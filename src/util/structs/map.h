/*****************************************************************/ /**
 * @file   map.h
 * @brief  Contains a Map template.
 *
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_MAP
#define HG_COLT_MAP

#include "vector.h"

namespace clt
{
  template<
      typename Key, typename Value, auto ALLOCATOR = mem::GlobalAllocatorDescription>
    requires meta::AllocatorScope<ALLOCATOR>
  /// @brief A unordered associative container that contains key/value pairs with unique keys.
  /// @tparam Key The Key that can be hashed through colt::hash or std::hash
  /// @tparam Value The Value that is accessed through the Key
  class Map
  {
    static_assert(meta::is_hashable_v<Key>, "Key of a Map should be hashable!");
    static_assert(
        std::equality_comparable<Key>, "Key of a Map should implement operator==!");

    /// @brief True if the allocator is global
    static constexpr bool is_global = meta::GlobalAllocator<ALLOCATOR>;
    /// @brief True if the allocator is local (we need a reference to it)
    static constexpr bool is_local = meta::LocalAllocator<ALLOCATOR>;

    /// @brief Type of the allocator
    using Allocator = mem::allocator_ref<ALLOCATOR>;

    /// @brief The allocator used for allocation/deallocation
    [[no_unique_address]] Allocator allocator;

  public:
    using Slot = typename std::pair<const Key, Value>;

  private:
    /// @brief Contains meta-data information about the slots of the map
    Vector<details::KeySentinel, ALLOCATOR> sentinel_metadata;
    /// @brief Memory block of the key/value pair
    mem::TypedBlock<Slot> slots = {};
    /// @brief The count of active objects in the container
    size_t size_v = 0;
    /// @brief The load factor before reallocation
    float load_factor = 0.70f;

    template<typename SlotT>
    /// @brief Map Iterator
    /// @tparam SlotT The Slot type of the Map (for const qualifiers)
    struct MapIterator
    {
      /// @brief Forward Iterator
      using iterator_category = std::forward_iterator_tag;
      /// @brief Value type of the iterator
      using value_type = SlotT;
      /// @brief Pointer type of the iterator
      using pointer = SlotT*;
      /// @brief Reference type of the iterator
      using reference = Slot&;

    private:
      /// @brief Pointer to the current active slot or end()
      SlotT* slot_ptr;
      /// @brief Pointer to the map from which the iterator was constructed
      meta::match_const_t<SlotT, Map>* map_ptr;

    public:
      /// @brief Constructor of MapIterator
      /// @param slot The active slot or end()
      /// @param map_ptr The pointer to the map
      constexpr MapIterator(
          SlotT* slot, meta::match_const_t<SlotT, Map>* map_ptr) noexcept
          : slot_ptr(slot)
          , map_ptr(map_ptr)
      {
      }

      /// @brief Returns a pointer to the current slot pointer to
      /// @return Current slot or end()
      constexpr SlotT* operator->() noexcept { return slot_ptr; }
      /// @brief Returns a pointer to the current slot pointer to
      /// @return Current slot or end()
      constexpr const SlotT* operator->() const noexcept { return slot_ptr; }
      /// @brief Returns a reference to the current slot pointer to
      /// @return Current slot or end()
      constexpr SlotT& operator*() noexcept { return *slot_ptr; }
      /// @brief Returns a reference to the current slot pointer to
      /// @return Current slot or end()
      constexpr const SlotT& operator*() const noexcept { return *slot_ptr; }

      /// @brief Increments the current iterator to the next active slot or end()
      /// @return Self
      constexpr MapIterator& operator++() noexcept
      {
        size_t index = slot_ptr - map_ptr->slots.ptr() + 1;
        for (size_t i = index; i < map_ptr->sentinel_metadata.size(); i++)
        {
          if (details::is_sentinel_active(map_ptr->sentinel_metadata[i]))
          {
            slot_ptr = map_ptr->slots.ptr() + i;
            return *this;
          }
        }
        //Set to end()
        slot_ptr = map_ptr->slots.ptr() + map_ptr->slots.size();
        return *this;
      }

      /// @brief Post increment operator
      /// @param  Post increment
      /// @return Current iterator
      constexpr MapIterator operator++(int) noexcept
      {
        MapIterator to_ret = *this; //copy
        ++(*this);                  //increment
        return to_ret;
      }

      /// @brief Check if two MapIterator are equal
      /// @param a First MapIterator
      /// @param b Second MapIterator
      /// @return True if equal
      friend constexpr bool operator==(
          const MapIterator& a, const MapIterator& b) noexcept = default;
    };

  public:
    /// @brief Constructs an empty Map, of load factor 0.7
    constexpr Map(float load_factor = 0.70f) noexcept
      requires is_global
        : sentinel_metadata(16, InPlace, details::EMPTY)
        , slots(allocator.alloc(16 * sizeof(Slot)))
        , load_factor(load_factor)
    {
      assert_true("Invalid load factor!", 0.0f < load_factor, load_factor < 1.0f);
    }

    /// @brief Constructs an empty Map, of load factor 0.7
    /// @param alloc The local allocator to make use of
    constexpr Map(Allocator& alloc, float load_factor = 0.70f) noexcept
      requires is_local
        : allocator(alloc)
        , sentinel_metadata(alloc, 16, InPlace, details::EMPTY)
        , slots(allocator.alloc(16 * sizeof(Slot)))
        , load_factor(load_factor)
    {
    }

    /// @brief Constructs a Map of load factor 0.7, reserving memory for 'reserve_size' objects
    /// @param reserve_size The count of object to reserve for
    constexpr Map(size_t reserve_size, float load_factor = 0.70f) noexcept
      requires is_global
        : sentinel_metadata(reserve_size, InPlace, details::EMPTY)
        , slots(allocator.alloc(reserve_size * sizeof(Slot)))
        , load_factor(load_factor)
    {
      assert_true("Invalid load factor!", 0.0f < load_factor, load_factor < 1.0f);
    }

    /// @brief Constructs a Map of load factor 0.7, reserving memory for 'reserve_size' objects
    /// @param reserve_size The count of object to reserve for
    constexpr Map(
        Allocator& alloc, size_t reserve_size, float load_factor = 0.70f) noexcept
      requires is_local
        : allocator(alloc)
        , sentinel_metadata(alloc, reserve_size, InPlace, details::EMPTY)
        , slots(allocator.alloc(reserve_size * sizeof(Slot)))
        , load_factor(load_factor)
    {
    }

    constexpr Map(const Map&) = delete;

    /// @brief Move constructs a map
    /// @param mp The map to move
    constexpr Map(Map&& mp) noexcept
        : sentinel_metadata(std::exchange(mp.sentinel_metadata, {}))
        , slots(std::exchange(mp.slots, {}))
        , load_factor(mp.load_factor)
    {
    }

    /// @brief Destructs a Map and its active elements
    ~Map() noexcept(
        std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Value>)
    {
      clear();
      allocator.dealloc(slots);
    }

    /// @brief Clear all the active elements in the Map
    constexpr void clear() noexcept(
        std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Value>)
    {
      for (size_t i = 0; i < sentinel_metadata.size(); i++)
      {
        if (details::is_sentinel_active(sentinel_metadata[i]))
          slots.ptr()[i].~Slot(); //destroy active slots
        sentinel_metadata[i] = details::EMPTY;
      }
      size_v = 0;
    }

    /// @brief Returns the number of active elements in the Map
    /// @return The count of active elements
    constexpr size_t size() const noexcept { return size_v; }

    /// @brief Returns the capacity of the current allocation
    /// @return The capacity of the current allocation
    constexpr size_t capacity() const noexcept { return slots.size(); }

    /// @brief Returns a MapIterator to the first active slot in the Map, or end() if no slots are active
    /// @return MapIterator to the first active slot or end()
    constexpr MapIterator<Slot> begin() noexcept
    {
      for (size_t i = 0; i < sentinel_metadata.size(); i++)
      {
        if (details::is_sentinel_active(sentinel_metadata[i]))
          return {slots.ptr() + i, this};
      }
      return end();
    }
    /// @brief Returns a MapIterator past the end of the Map
    /// @return MapIterator that should not be dereferenced
    constexpr MapIterator<Slot> end() noexcept
    {
      return {slots.ptr() + slots.size(), this};
    }
    /// @brief Returns a MapIterator to the first active slot in the Map, or end() if no slots are active
    /// @return MapIterator to the first active slot or end()
    constexpr MapIterator<const Slot> begin() const noexcept
    {
      for (size_t i = 0; i < sentinel_metadata.size(); i++)
      {
        if (details::is_sentinel_active(sentinel_metadata[i]))
          return {slots.ptr() + i, this};
      }
      return end();
    }
    /// @brief Returns a MapIterator past the end of the Map
    /// @return MapIterator that should not be dereferenced
    constexpr MapIterator<const Slot> end() const noexcept
    {
      return {slots.ptr() + slots.size(), this};
    }

    /// @brief Check if the Map is empty
    /// @return True if the Map is empty
    constexpr bool is_empty() const noexcept { return size_v == 0; }

    /// @brief Check if the Map will reallocate on the next call of insert/insertOrAssign
    /// @return True if the Map will reallocate
    constexpr bool will_reallocate() const noexcept
    {
      return float(size_v + 1) > load_factor * capacity();
    }

    /// @brief Returns the load factor of the Map
    /// @return The load factor
    constexpr float get_load_factor() const noexcept { return load_factor; }

    /// @brief Sets the load factor to 'nload_factor'.
    /// Precondition: nload_factor < 1.0f && nload_factor > 0.0f.
    /// @param nload_factor The new load factor
    constexpr void set_load_factor(float nload_factor) noexcept
    {
      assert_true("Invalid load factor!", 0.0f < load_factor, load_factor < 1.0f);
      load_factor = nload_factor;
    }

    /// @brief Finds the key/value pair of key 'key'
    /// @param key The key to search for
    /// @return Pointer to the key/value pair if found, or null
    constexpr const Slot* find(const Key& key) const noexcept
    {
      const size_t key_hash = hash_value(key);
      size_t prob_index     = key_hash % slots.size();
      for (;;)
      {
        if (auto sentinel = sentinel_metadata[prob_index];
            details::is_sentinel_empty(sentinel))
          return nullptr; //not found
        else if (details::is_sentinel_deleted(sentinel))
        {
          prob_index = details::advance_prob(prob_index, slots.size());
          continue;
        }
        else if (
            details::is_sentinel_equal(sentinel, key_hash)
            && slots.ptr()[prob_index].first == key)
          return slots.ptr() + prob_index;
        prob_index = details::advance_prob(prob_index, slots.size());
      }
    }

    /// @brief Finds the key/value pair of key 'key'
    /// @param key The key to search for
    /// @return Pointer to the key/value pair if found, or null
    constexpr Slot* find(const Key& key) noexcept
    {
      //No UB as the map is not const
      return const_cast<Slot*>(static_cast<const Map*>(this)->find(key));
    }

    /// @brief Check if the Map contains a key/value pair of key 'key'.
    /// Prefer using 'find' if the value which is being checked for will be used.
    /// @param key The key to check for
    /// @return True if the Map contains 'key' else false
    constexpr bool contains(const Key& key) const noexcept
    {
      return find(key) != nullptr;
    }

    /// @brief Inserts a new value if 'key' does not already exist.
    /// Returns an InsertionResult SUCCESS (if the insertion was performed) or EXISTS (if the key already exists).
    /// The returned pointer is to the newly inserted key/value on SUCCESS.
    /// The returned pointer is to the existing key/value that matches 'key' on EXISTS.
    /// The returned pointer is never null.
    /// @param key The key of the value 'value'
    /// @param value The value to insert
    /// @return Pair of pointer to the inserted slot or the existent one, and SUCESS on insertion or EXISTS if the key already exists
    constexpr std::pair<Slot*, InsertionResult> insert(const Key& key, const Value& value) noexcept(
        std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Value>
        && std::is_nothrow_move_constructible_v<Key>
        && std::is_nothrow_move_constructible_v<Value>
        && std::is_nothrow_copy_constructible_v<Key>
        && std::is_nothrow_copy_constructible_v<Value>)
    {
      if (will_reallocate())
        realloc_map(capacity() + 16);

      const size_t key_hash = hash_value(key);
      size_t prob_index;
      if (find_key(key_hash, key, prob_index, sentinel_metadata, slots))
      {
        new (slots.ptr() + prob_index) Slot(key, value);
        //Set the slot to ACTIVE
        sentinel_metadata[prob_index] = details::create_active_sentinel(key_hash);
        //Update size
        ++size_v;
        return {slots.ptr() + prob_index, InsertionResult::SUCCESS};
      }
      else
        return {slots.ptr() + prob_index, InsertionResult::EXISTS};
    }

    /// @brief Inserts a new value if 'key' does not already exist.
    /// Returns an InsertionResult SUCCESS (if the insertion was performed) or EXISTS (if the key already exists).
    /// The returned pointer is to the newly inserted key/value on SUCCESS.
    /// The returned pointer is to the existing key/value that matches 'key' on EXISTS.
    /// The returned pointer is never null.
    /// @param key The key of the value 'value'
    /// @param value The value to insert
    /// @return Pair of pointer to the inserted slot or the existent one, and SUCESS on insertion or EXISTS if the key already exists
    constexpr std::pair<Slot*, InsertionResult> insert(const Key& key, Value&& value) noexcept(
        std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Value>
        && std::is_nothrow_move_constructible_v<Key>
        && std::is_nothrow_move_constructible_v<Value>
        && std::is_nothrow_copy_constructible_v<Key>
        && std::is_nothrow_copy_constructible_v<Value>)
    {
      if (will_reallocate())
        realloc_map(capacity() + 16);

      const size_t key_hash = hash_value(key);
      size_t prob_index;
      if (find_key(key_hash, key, prob_index, sentinel_metadata, slots))
      {
        new (slots.ptr() + prob_index) Slot(key, std::move(value));
        //Set the slot to ACTIVE
        sentinel_metadata[prob_index] = details::create_active_sentinel(key_hash);
        //Update size
        ++size_v;
        return {slots.ptr() + prob_index, InsertionResult::SUCCESS};
      }
      else
        return {slots.ptr() + prob_index, InsertionResult::EXISTS};
    }

    /// @brief Insert a new value if 'key' does not already exist, else assigns 'value' to the existing value.
    /// Returns an InsertionResult SUCCESS (if the insertion was performed) or ASSIGNED (if the key already exists and was assigned).
    /// The returned pointer is to the newly inserted key/value on SUCCESS.
    /// The returned pointer is to the existing key/value that matches 'key' on ASSIGNED.
    /// The returned pointer is never null.
    /// @param key The key of the value 'value'
    /// @param value The value to insert or assign
    /// @return Pair of pointer to the inserted/assigned slot, and SUCESS on insertion or ASSIGN on assignment
    constexpr std::pair<Slot*, InsertionResult> insert_or_assign(const Key& key, const Value& value) noexcept(
        std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Value>
        && std::is_nothrow_move_constructible_v<Key>
        && std::is_nothrow_move_constructible_v<Value>
        && std::is_nothrow_copy_constructible_v<Key>
        && std::is_nothrow_copy_constructible_v<Value>
        && std::is_nothrow_copy_assignable_v<Value>)
    {
      if (will_reallocate())
        realloc_map(capacity() + 16);

      const size_t key_hash = hash_value(key);
      size_t prob_index;
      if (find_key(key_hash, key, prob_index, sentinel_metadata, slots))
      {
        new (slots.ptr() + prob_index) Slot(key, value);
        //Set the slot to ACTIVE
        sentinel_metadata[prob_index] = details::create_active_sentinel(key_hash);
        //Update size
        ++size_v;
        return {slots.ptr() + prob_index, InsertionResult::SUCCESS};
      }
      else
      {
        slots.ptr()[prob_index].second = value;
        return {slots.ptr() + prob_index, InsertionResult::ASSIGNED};
      }
    }

    /// @brief Erases a key if it exists
    /// @param key The key whose key/value pair to erase
    /// @return True if the key existed and was erased, else false
    constexpr bool erase(const Key& key) noexcept(
        std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Value>)
    {
      if (Slot* ptr = find(key))
      {
        size_t index             = ptr - slots.ptr();
        sentinel_metadata[index] = details::DELETED; //set the sentinel to deleted
        ptr->~Slot();                                //destroy the key/value pair
        //Update size
        --size_v;
        return true;
      }
      else
        return false;
    }

    constexpr void reserve(size_t by_more) noexcept(
        std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Value>
        && std::is_nothrow_move_constructible_v<Key>
        && std::is_nothrow_move_constructible_v<Value>)
    {
      realloc_map(by_more);
    }

    /// @brief Calls 'find' on 'key'
    /// @param key The key to search for
    /// @return Pointer to the found slot or null if not found
    constexpr const Slot* operator[](const Key& key) const noexcept
    {
      return find(key);
    }

    /// @brief Calls 'find' on 'key'
    /// @param key The key to search for
    /// @return Pointer to the found slot or null if not found
    constexpr Slot* operator[](const Key& key) noexcept { return find(key); }

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
    static constexpr bool find_key(
        size_t key_hash, const Key& key, size_t& prob,
        const Vector<details::KeySentinel>& metadata,
        mem::TypedBlock<Slot> blk) noexcept
    {
      assert_true("Wrong use of API", key_hash == hash_value(key));
      assert_true("Wrong use of API", metadata.size() == blk.size());
      size_t prob_index = key_hash % blk.size();
      for (;;)
      {
        if (auto sentinel = metadata[prob_index];
            details::is_sentinel_empty(sentinel)
            || details::is_sentinel_deleted(sentinel))
        {
          prob = prob_index;
          return true;
        }
        else if (
            details::is_sentinel_equal(sentinel, key_hash)
            && blk.ptr()[prob_index].first == key)
        {
          prob = prob_index;
          return false;
        }
        prob_index = details::advance_prob(prob_index, blk.size());
      }
    }

    /// @brief Augments the capacity of the Map, rehashing in the process
    /// @param new_capacity The new capacity of the map
    constexpr void realloc_map(size_t new_capacity) noexcept(
        std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Value>
        && std::is_nothrow_move_constructible_v<Key>
        && std::is_nothrow_move_constructible_v<Value>)
    {
      mem::TypedBlock<Slot> new_slot = allocator.alloc(new_capacity * sizeof(Slot));

      Vector<details::KeySentinel> new_metadata = {
          new_capacity, InPlace, details::EMPTY};
      for (size_t i = 0; i < sentinel_metadata.size(); i++)
      {
        auto sentinel = sentinel_metadata[i];
        if (details::is_sentinel_active(sentinel))
        {
          //find the key
          Slot* ptr             = slots.ptr() + i;
          const size_t key_hash = hash_value(ptr->first);
          size_t prob_index;
          //Rehash the key to get its new index in the new array
          if (find_key(key_hash, ptr->first, prob_index, new_metadata, new_slot))
          {
            //Move destruct
            new (new_slot.ptr() + prob_index) Slot(std::move(*ptr));
            ptr->~Slot();

            //Set the slot to ACTIVE
            new_metadata[prob_index] = details::create_active_sentinel(key_hash);
          }
        }
      }
      sentinel_metadata = std::move(new_metadata);
      allocator.dealloc(slots);
      slots = new_slot;
    }
  };
} // namespace clt

template<typename Key, typename Value, auto ALLOCATOR>
  requires clt::meta::AllocatorScope<ALLOCATOR> && fmt::is_formattable<Key>::value
           && fmt::is_formattable<Value>::value
struct fmt::formatter<clt::Map<Key, Value, ALLOCATOR>>
{
  template<typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    auto it  = ctx.begin();
    auto end = ctx.end();
    assert_true("Possible format for Map is: {}!", it == end);
    return it;
  }

  template<typename FormatContext>
  auto format(const clt::Map<Key, Value, ALLOCATOR>& vec, FormatContext& ctx)
  {
    auto fmt_to = ctx.out();
    if (vec.is_empty())
      return fmt_to;
    auto it = vec.begin();
    for (size_t i = 0; i < vec.size() - 1; i++)
    {
      fmt_to = fmt::format_to(fmt_to, "{{ {}, {} }}, ", it->first, it->second);
      ++it;
    }
    fmt_to = fmt::format_to(fmt_to, "{{ {}, {} }}", it->first, it->second);
    return fmt_to;
  }
};

#endif //!HG_COLT_MAP