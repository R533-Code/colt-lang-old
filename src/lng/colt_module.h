#ifndef HG_COLT_MODULE
#define HG_COLT_MODULE

#include <concepts>

#include "colt_module_name.h"
#include "structs/map.h"
#include "lng/colt_global.h"

namespace clt::lng
{
  // Forward declarations
  class ModuleBuffer;
  class Module;

  class ModuleToken
  {
    /// @brief The module index
    u32 module_nb;
    /// @brief The module depth (nesting level)
    u8 nesting;

    friend class ModuleBuffer;
    friend class Module;
  
    constexpr ModuleToken(u32 module_nb, u8 nesting) noexcept
      : module_nb(module_nb), nesting(nesting) {}

    /// @brief Returns an invalid module token
    /// @return An invalid module token
    static constexpr ModuleToken getInvalid() noexcept
    {
      return ModuleToken{ std::numeric_limits<u32>::max(), 0 };
    }

    /// @brief Check if the current ModuleToken is invalid
    /// @return True if invalid
    constexpr bool isInvalid() const noexcept
    {
      return module_nb == std::numeric_limits<u32>::max();
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
    /// @return True if the ModuleToken represents the global module
    constexpr bool isGlobalModule() const noexcept
    {
      return module_nb == 0;
    }

    /// @brief Check if this module cannot have a submodule
    /// @return True if this module cannot have a submodule
    constexpr bool isLeaf() const noexcept { return nesting == ModuleName::max_size(); }
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
    bool isGlobal() const noexcept { return parent.isInvalid(); }

    /// @brief Returns the ModuleToken representing the parent of this module
    /// @return The ModuleToken representing the parent of this module
    ModuleToken getParent() const noexcept
    {
      assert_true("Global module does not have a parent!", isGlobal());
      return parent;
    }

    /// @brief Check if this module cannot have a submodule
    /// @return True if this module cannot have a submodule
    bool isLeaf() const noexcept { return parent.nesting + 1 == ModuleName::max_size(); }
  };

  /// @brief Class responsible of storing modules
  class ModuleBuffer
  {
    FlatList<Module> modules{};

  public:
    /// @brief Constructs a module buffer
    ModuleBuffer()
    {
      // Add global module: 0
      modules.push_back(InPlace, "", ModuleToken::getInvalid());
    }

    ModuleBuffer(ModuleBuffer&&) noexcept = default;

    /// @brief Returns the token representing the global module
    /// @return The token representing the global module
    static ModuleToken getGlobalToken() noexcept { return ModuleToken::getGlobalModule(); }

    /// @brief Returns the module represented by ModuleToken
    /// @param tkn The module token
    /// @return Module represented by ModuleToken
    Module& getModule(ModuleToken tkn) noexcept { return modules[tkn.module_nb]; }
    /// @brief Returns the module represented by ModuleToken
    /// @param tkn The module token
    /// @return Module represented by ModuleToken
    const Module& getModule(ModuleToken tkn) const noexcept { return modules[tkn.module_nb]; }
    
    /// @brief Returns the global module
    /// @return Global module
    Module& getGlobalModule() noexcept { return modules[0]; }
    /// @brief Returns the global module
    /// @return Global module
    const Module& getGlobalModule() const noexcept { return modules[0]; }

    /// @brief Adds a submodule to a module
    /// @param add_to The module to which to add a submodule
    /// @param submodule The submodule to add
    void addSubmodule(ModuleToken add_to, ModuleToken submodule) noexcept
    {
      modules[add_to.module_nb].addSubmodule(submodule);
    }

    /// @brief Creates a module
    /// @param name The name of the module
    /// @param parent The parent of the module
    /// @return The token representing the module
    Option<ModuleToken> createModule(StringView name, ModuleToken parent = getGlobalToken()) noexcept
    {
      assert_true("Invalid name for module!", !name.empty());
      assert_true("Integer overflow!", modules.size() < std::numeric_limits<u32>::max());
      if (parent.isLeaf())
        return None;
      ModuleToken tkn = { static_cast<u32>(modules.size()), static_cast<u8>(parent.nesting + 1) };
      modules.push_back(InPlace, name, parent);
      return tkn;
    }
    
    /// @brief Returns the parent of the module represented by 'tkn'
    /// @param tkn The module whose parent to return
    /// @return The parent of the module
    const Module& getParent(ModuleToken tkn) const noexcept
    {
      assert_true("Global module does not have a parent!", !tkn.isGlobalModule());
      return modules[getModule(tkn).getParent().module_nb];
    }

    /// @brief Returns the parent of the module represented by 'tkn'
    /// @param tkn The module whose parent to return
    /// @return The parent of the module
    Module& getParent(ModuleToken tkn) noexcept
    {
      assert_true("Global module does not have a parent!", !tkn.isGlobalModule());
      return modules[getModule(tkn).getParent().module_nb];
    }
  };

  template<std::forward_iterator It>
  constexpr ModuleName::ModuleName(It begin, It end) noexcept
  {
    assert_true("Name of non-global module must at least be of size 1!", begin != end);
    for (; name_size < MAX_NESTING_LEVEL; name_size++)
    {
      if (begin != end)
        name[name_size] = *begin++;
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