#ifndef HG_CLT_DYNLOAD
#define HG_CLT_DYNLOAD

#include <dynload/dynload.h>
#include <common/assertions.h>
#include <common/types.h>
#include <structs/option.h>

namespace clt::run
{
  /// @brief Represents a dynamically loaded library
  class DynamicLibrary
  {
    RAIIResource<DLLib, &dlFreeLibrary> lib;
    RAIIResource<DLSyms, &dlSymsCleanup> syms;

    DynamicLibrary(DLLib* lib, DLSyms* syms) noexcept
        : lib(lib)
        , syms(syms)
    {
    }

  public:
    DynamicLibrary(const DynamicLibrary&)                = delete;
    DynamicLibrary(DynamicLibrary&& other) noexcept      = default;
    DynamicLibrary& operator=(const DynamicLibrary&)     = delete;
    DynamicLibrary& operator=(DynamicLibrary&&) noexcept = default;

    /// @brief Loads library at path 'path'
    /// @param path The path (not null)
    /// @return None on errors or handle to the library
    static Option<DynamicLibrary> load(const char* path) noexcept;    

    /// @brief Loads the current executable as a library
    /// @return None on errors or handle to the current library
    static Option<DynamicLibrary> load_current() noexcept;    

    /// @brief Searches for symbol of name 'name'
    /// @param name The name of the symbol (must be mangled for C++ symbols)
    /// @return Pointer to the symbol or nullptr
    void* lookup(const char* name) noexcept { return dlFindSymbol(lib.get(), name); }

    /// @brief Returns the count of symbols in the current library
    /// @return The count of symbols in the current library
    u64 count() const noexcept { return (u64)dlSymsCount(syms.get()); }

    /// @brief Returns the name of the symbol at address 'symbol'
    /// @param symbol The symbol's address
    /// @return The name or None on errors
    Option<const char*> name(void* symbol) const noexcept
    {
      auto ptr = dlSymsNameFromValue(syms.get(), symbol);
      if (ptr == nullptr)
        return None;
      return ptr;
    }

    /// @brief Returns the name of the symbol 'index'
    /// @param index The index of the symbol (< count())
    /// @return The name of the symbol 'index'
    Option<const char*> name(u64 index) const noexcept
    {
      assert_true("Invalid index!", index < count());
      auto ptr = dlSymsName(syms.get(), (i32)index);
      if (ptr == nullptr)
        return None;
      return ptr;
    }

    /// @brief Iterator over symbols of a library
    class iterator
    {
      /// @brief The symbols
      DLSyms* syms;
      /// @brief The current index
      i32 current;

    public:
      /// @brief Constructor
      /// @param syms The symbols (not null)
      /// @param starting_index The starting index
      iterator(DLSyms* syms, i32 starting_index = 0) noexcept
          : syms(syms)
          , current(starting_index)
      {
      }

      /// @brief Dereferences the iterator
      /// @return The current symbol name
      const char* operator*() const noexcept
      {
        assert_true("Invalid iterator!", current < dlSymsCount(syms));
        return dlSymsName(syms, current);
      }

      /// @brief Dereferences the iterator
      /// @return The current symbol name
      const char* operator->() const noexcept { return **this; }

      /// @brief Advances the iterator
      /// @return Self
      iterator& operator++() noexcept
      {
        current++;
        return *this;
      }

      /// @brief Advances the iterator
      /// @return Old iterator value
      iterator operator++(int) noexcept
      {
        iterator tmp = *this;
        ++(*this);
        return tmp;
      }

      /// @brief Go to previous value of the iterator
      /// @return Self
      iterator& operator--() noexcept
      {
        assert_true("Invalid iterator!", current != 0);
        current++;
        return *this;
      }

      /// @brief Go to previous value of the iterator
      /// @return Old iterator value
      iterator operator--(int) noexcept
      {
        iterator tmp = *this;
        --(*this);
        return tmp;
      }

      /// @brief Comparison operator
      /// @return true if equal
      friend bool operator==(const iterator&, const iterator&) = default;
    };

    /// @brief Iterator over symbols
    /// @return Start iterator
    iterator begin() const noexcept { return iterator(syms.get(), 0); }
    /// @brief Iterator over symbols
    /// @return End iterator
    iterator end() const noexcept
    {
      return iterator(syms.get(), dlSymsCount(syms.get()));
    }
  };
} // namespace clt::run

#endif // !HG_CLT_DYNLOAD
