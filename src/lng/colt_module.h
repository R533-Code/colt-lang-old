#ifndef HG_COLT_MODULE
#define HG_COLT_MODULE

#include <concepts>

#include "colt_module_name.h"
#include "structs/map.h"
#include "colt_global.h"

namespace clt::lng
{
  // Forward declarations
  class ModuleBuffer;
  class Module;

  class ModuleToken
  {
    /// @brief The module nesting level
    u8 module_level;
    u8 module_nb;

    friend class ModuleBuffer;
    friend class Module;
  
    constexpr ModuleToken(u8 module_nb, u8 module_level) noexcept
      : module_level(module_level), module_nb(module_nb) {}

    /// @brief Returns an invalid module token
    /// @return An invalid module token
    static constexpr ModuleToken getInvalid() noexcept
    {
      return ModuleToken{ ModuleName::max_size(), 0 };
    }

    /// @brief Check if the current ModuleToken is invalid
    /// @return True if invalid
    constexpr bool isInvalid() const noexcept
    {
      return module_level == ModuleName::max_size();
    }
  
  public:
    ModuleToken() = delete;
    constexpr ModuleToken(ModuleToken&&) noexcept = default;
    constexpr ModuleToken(const ModuleToken&) noexcept = default;
    constexpr ModuleToken& operator=(ModuleToken&&) noexcept = default;
    constexpr ModuleToken& operator=(const ModuleToken&) noexcept = default;

    /// @brief Returns a token representing the global module
    /// @return ModuleToken representing the global module
    static constexpr ModuleToken getGlobalModule() noexcept
    {
      return ModuleToken{ 0, 0 };
    }    

    /// @brief Check if the current module represents the global module
    /// @return 
    constexpr bool isGlobalModule() const noexcept
    {
      return module_nb == 0 && module_level == 0;
    }
  };

  /// @brief Represents a colt module.
  /// A module has a name, a parent (std::numeric has parent std::)
  /// and a global table, which contains all global identifiers
  /// (types, functions...).
  class Module
  {
    /// @brief The name of the module
    StringView module_name;
    /// @brief The parent of the current module (null for global module)
    ModuleToken parent;
    // TODO: Replace with SmallVector
    /// @brief The submodules of the current module
    Vector<ModuleToken> submodules{};
    /// @brief The table of globals contained in the current module
    Map<StringView, GlobalVariant> global_table{};

  public:
    Module() = delete;
    Module(Module&&) noexcept = default;

    /// @brief Constructs a Module
    /// @param name The name of the module
    /// @param parent The module's parent
    Module(StringView name, ModuleToken parent) noexcept
      : module_name(name), parent(parent) {}

    /// @brief Adds a submodule for this module
    /// @param mod The submodule to add
    void addSubmodule(ModuleToken mod) noexcept
    {
      submodules.push_back(mod);
    }

    /// @brief Check if the current module is the global module
    /// @return True if parent
    bool isGlobal() const noexcept { return parent.isGlobalModule(); }
  };

  class ModuleBuffer
  {
    // Realistically, more than 8 nested submodules is not likely.
    // Moreover, more than 256 modules on the same level is also not likely.
    // Thus, to represent the parent's relationship of the modules,
    // we will use indices into vectors.
    
    std::array<Vector<Module>, 8> modules;

  public:

  };  

  template<std::forward_iterator It>
  constexpr ModuleName::ModuleName(It begin, It end) noexcept
  {
    assert_true("Name of non-global module must at least be of size 1!", begin != end);
    for (; name_size < MAX_NESTING_LEVEL; name_size++)
    {
      if (begin != end)
        name[i] = *begin++;
      else
        break;
    }
    // Verify the name format
    if constexpr (isDebugBuild())
    {
      bool hit_empty = false;
      for (size_t i = 0; i < MAX_NESTING_LEVEL; i++)
      {
        if (name[i] == "")
          hit_empty = true;
        else
          assert_true("Invalid name for module! An empty slot must not be followed by a non-empty one!", !hit_empty);
      }
    }
  }

  constexpr bool ModuleName::operator==(const ModuleName& other) const noexcept
  {
    if (other.size() != size())
      return false;
    for (size_t i = 0; i < size(); i++)
      if (other[i] != name[i])
        return false;
    return true;
  }

  constexpr ModuleName ModuleName::addSubmodule(StringView name) const noexcept
  {
    assert_true("Module name is already full!", size() != max_size());
    ModuleName cpy = *this;
    cpy.name[cpy.name_size++] = name;
    return cpy;
  }
}

template<>
struct clt::hash<clt::lng::ModuleName>
{
  constexpr size_t operator()(const clt::lng::ModuleName& name) const noexcept
  {
    return hash_value(name.to_view());
  }
};

#endif // !HG_COLT_MODULE