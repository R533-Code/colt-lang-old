/*****************************************************************/ /**
 * @file   colt_module_name.h
 * @brief  Contains ModuleName, which represents a colt module name.
 *
 * @author RPC
 * @date   March 2024
 *********************************************************************/
#ifndef HG_COLT_MODULE_NAME
#define HG_COLT_MODULE_NAME

#include <concepts>

#include "structs/vector.h"
#include "common/types.h"
#include "common/assertions.h"

namespace clt::lng
{
  /// @brief Represents a Module name
  class ModuleName
  {
    /// @brief The maximum nesting allowed in module name
    static constexpr u8 MAX_NESTING_LEVEL = 4;

    /// @brief The names
    std::array<StringView, MAX_NESTING_LEVEL> name{};
    /// @brief The number of slot active in the name array
    size_t name_size = 0;

    constexpr ModuleName() noexcept = default;

  public:
    constexpr ModuleName(ModuleName&&) noexcept                 = default;
    constexpr ModuleName(const ModuleName&) noexcept            = default;
    constexpr ModuleName& operator=(ModuleName&&) noexcept      = default;
    constexpr ModuleName& operator=(const ModuleName&) noexcept = default;

    template<std::forward_iterator It>
    /// @brief Constructs a module name using iterators.
    /// The name will be formed using the first 'max_size()' values
    /// produced by the iterators.
    constexpr ModuleName(It begin, It end) noexcept;

    /// @brief Check if the current name represents the global module name.
    /// The global module name is empty
    /// @return True if size() == 0
    constexpr bool is_global() const noexcept { return name_size == 0; }

    /// @brief The maximum size of a module name
    /// @return The max size of a module name
    constexpr static u8 max_size() noexcept { return MAX_NESTING_LEVEL; }

    /// @brief The size of the current module name
    /// @return The size of the module name
    constexpr size_t size() const noexcept { return name_size; }

    /// @brief Returns an iterator to the beginning of the name
    /// @return Iterator to the beginning of the name
    constexpr auto begin() const noexcept { return name.begin(); }
    /// @brief Returns an iterator past the end of the name
    /// @return Iterator to the end of the name
    constexpr auto end() const noexcept { return name.begin() + name_size; }

    /// @brief Returns the module name at nesting 'level'.
    /// As an example std::io -> [0]=std, [1]=io.
    /// @return The module name at level 'index'
    constexpr StringView operator[](size_t index) const noexcept
    {
      assert_true("Invalid index for Module name!", index < name_size);
      return name[index];
    }

    /// @brief Returns a view over the name
    /// @return View over the name
    constexpr View<StringView> to_view() const noexcept
    {
      return {name.data(), name_size};
    }

    /// @brief Compares two module names
    /// @param other The other module name
    /// @return True if both name represent the same module
    constexpr bool operator==(const ModuleName& other) const noexcept;

    /// @brief Creates a new module name by adding 'name' to the current name.
    /// @param name The name to add to the end of the current name
    /// @return The new module name
    /// @pre size() != max_size()
    constexpr ModuleName add_submodule(StringView name) const noexcept;

    /// @brief Returns the global module name.
    /// The global module is the only name whose size is zero.
    /// @return The global module name
    constexpr static ModuleName global_module() noexcept { return ModuleName{}; }
  };
} // namespace clt::lng

#endif // !HG_COLT_MODULE_NAME