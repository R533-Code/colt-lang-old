#ifndef HG_COLT_GLOBAL
#define HG_COLT_GLOBAL

#include "lng/macro_helper.h"

DECLARE_ENUM_WITH_TYPE(u8, clt::lng, GlobalID,
  GLOBAL_FN,
  GLOBAL_VAR,
  GLOBAL_TYPE,
  GLOBAL_ALIAS
);

/// @brief Macro Type List (with same index as GlobalID declaration!)
#define COLTC_GLOBAL_LIST FnGlobal, VarGlobal, TypeGlobal, AliasGlobal

namespace clt::lng
{
  // Forward declarations
  class GlobalVariant;
  FORWARD_DECLARE_TYPE_LIST(COLTC_GLOBAL_LIST);
  // TypeToGlobalID
  CONVERT_TYPE_TO_ENUM(GlobalID, COLTC_GLOBAL_LIST);

  /// @brief Base class of all types
  class GlobalBase
  {
  public:
    static constexpr u64 BITS_FOR_ID = 7;

    static_assert(clt::reflect<GlobalID>::max() < (2 << BITS_FOR_ID),
      "Not enough bits to represent GlobalID!");

  private:
    /// @brief The ID of the type (used for down-casts)
    u8 global_id : BITS_FOR_ID;
    /// @brief True if private, false if public
    u8 isprivate : 1;

  public:
    // No default constructor
    constexpr GlobalBase() noexcept = delete;
    /// @brief Constructs a GlobalBase
    /// @param id The Global ID
    constexpr GlobalBase(GlobalID id, bool is_private) noexcept
      : global_id(static_cast<u8>(id)), isprivate(is_private) {}
    MAKE_DEFAULT_COPY_AND_MOVE_FOR(GlobalBase);

    /// @brief Check if the current global is private
    /// @return True if private
    constexpr bool isPrivate() const noexcept { return isprivate; }
    /// @brief Check if the current global is public
    /// @return True if public
    constexpr bool isPublic() const noexcept { return !isprivate; }

    /// @brief Returns the ID of the type (used for down-casts)
    /// @return The GlobalID of the current type
    constexpr GlobalID classof() const noexcept { return static_cast<GlobalID>(global_id); }

    /// @brief Check if the Global is a function
    /// @return True if function
    constexpr bool isFn() const noexcept { return classof() == GlobalID::GLOBAL_FN; }
    /// @brief Check if the Global is a global variable
    /// @return True if variable
    constexpr bool isVar() const noexcept { return classof() == GlobalID::GLOBAL_VAR; }
    /// @brief Check if the Global is a type
    /// @return True if type
    constexpr bool isType() const noexcept { return classof() == GlobalID::GLOBAL_TYPE; }
    /// @brief Check if the Global is an alias
    /// @return True if an alias
    constexpr bool isAlias() const noexcept { return classof() == GlobalID::GLOBAL_ALIAS; }
  };

  template<typename T>
  /// @brief A ColtType provides its ID and is equality comparable
  concept ColtGlobal = std::equality_comparable<T> && std::convertible_to<T, GlobalBase>&& requires (T a)
  {
    { a.classof() } -> std::same_as<GlobalID>;
  };

  /// @brief Represents a function
  class FnGlobal
    final : public GlobalBase
  {
  public:
    /// @brief Constructs a global variable
    /// @param is_private True if private
    constexpr FnGlobal(bool is_private) noexcept
      : GlobalBase(TypeToGlobalID<FnGlobal>(), is_private) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(FnGlobal);
  };

  /// @brief Represents a global variable
  class VarGlobal
    final : public GlobalBase
  {
  public:
    /// @brief Constructs a global variable
    /// @param is_private True if private
    constexpr VarGlobal(bool is_private) noexcept
      : GlobalBase(TypeToGlobalID<VarGlobal>(), is_private) {}
    
    MAKE_DEFAULT_COPY_AND_MOVE_FOR(VarGlobal);
  };

  /// @brief Represents a custom type
  class TypeGlobal
    final : public GlobalBase
  {
  public:
    /// @brief Constructs a global type
    /// @param is_private True if private
    constexpr TypeGlobal(bool is_private) noexcept
      : GlobalBase(TypeToGlobalID<TypeGlobal>(), is_private) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(TypeGlobal);
  };

  /// @brief Represents an alias to another global
  class AliasGlobal
    final : public GlobalBase
  {
    /// @brief The aliased global
    GlobalVariant* alias_to;

  public:
    /// @brief Constructs an alias to another global
    /// @param alias_to The aliased global
    /// @param is_private True if private
    constexpr AliasGlobal(GlobalVariant& alias_to, bool is_private) noexcept
      : GlobalBase(TypeToGlobalID<AliasGlobal>(), is_private), alias_to(&alias_to) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(AliasGlobal);

    /// @brief Returns the aliased global
    /// @return The aliased global
    constexpr GlobalVariant& getAliasTo() const noexcept { return *alias_to; }
  };

  /// @brief Macro used to generate getUnionMember body
#define COLTC_getUnionMember \
  if constexpr (std::same_as<T, FnGlobal>) \
    return fn_global; \
  if constexpr (std::same_as<T, VarGlobal>) \
    return var_global; \
  if constexpr (std::same_as<T, TypeGlobal>) \
    return type_global; \
  if constexpr (std::same_as<T, AliasGlobal>) \
    return alias_global;

  /// @brief Represents a global.
  /// Rather than using an inheritance, we make use of a variant.
  class GlobalVariant
  {
    template<typename T, typename... Args>
    /// @brief Helper to construct the union
    /// @param placement The pointer to the object to construct
    /// @param args... The arguments to forward to the constructor
    /// @return Empty monostate
    static constexpr std::monostate construct(T* placement, Args&&... args)
    {
      // Use construct_at rather than placement new for constexpr
      std::construct_at(placement, std::forward<Args>(args)...);
      return {};
    }

    /// @brief The possible types contained in the variant
    union
    {
      // Helper used in constructor
      std::monostate   _mono_state_;
      /// @brief The function global
      FnGlobal         fn_global;
      /// @brief The variable global
      VarGlobal        var_global;
      /// @brief The type global
      TypeGlobal       type_global;
      /// @brief The alias global
      AliasGlobal      alias_global;
    };

    template<typename T>
    /// @brief Returns a reference to the union member of type 'T'
    /// @return Reference to the union member of type 'T'
    constexpr const auto& getUnionMember() const noexcept { COLTC_getUnionMember; }

    template<typename T>
    /// @brief Returns a reference to the union member of type 'T'
    /// @return Reference to the union member of type 'T'
    constexpr auto& getUnionMember() noexcept { COLTC_getUnionMember; }

  public:
    template<ColtGlobal Type, typename... Args>
    /// @brief Constructor
    /// @param args... The arguments to forward to the constructor
    constexpr GlobalVariant(std::type_identity<Type>, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<Type, Args...>)
      : _mono_state_(construct<Type>(&getUnionMember<Type>(), std::forward<Args>(args)...))
    {
      // The ugly code above is used to keep the constructor constexpr
    }

    constexpr GlobalVariant(GlobalVariant&&) noexcept = default;
    constexpr GlobalVariant(const GlobalVariant&) noexcept = default;
    constexpr GlobalVariant& operator=(GlobalVariant&&) noexcept = default;
    constexpr GlobalVariant& operator=(const GlobalVariant&) noexcept = default;

    /// @brief Returns the global ID of the current type
    /// @return The ID of the current type
    constexpr GlobalID getGlobalID() const noexcept
    {
      // No idea if this is UB, but for know leave it as is.
      return std::bit_cast<GlobalBase>(fn_global).classof();
    }

    /// @brief Check if this global is private
    /// @return True if private
    constexpr bool isPrivate() const noexcept
    {
      return std::bit_cast<GlobalBase>(fn_global).isPrivate();
    }

    /// @brief Check if this global is public
    /// @return True if public
    constexpr bool isPublic() const noexcept { return !isPrivate(); }

    template<ColtGlobal T>
    /// @brief Casts the current type to 'T'
    /// @tparam T The type to cast to
    /// @return nullptr if the variant does not contain 'T', or valid pointer to 'T'
    constexpr T* getGlobal() noexcept
    {
      if (getGlobalID() != TypeToGlobalID<T>())
        return nullptr;
      return &getUnionMember<T>();
    }
    
    template<ColtGlobal T>
    /// @brief Casts the current type to 'T'
    /// @tparam T The type to cast to
    /// @return nullptr if the variant does not contain 'T', or valid pointer to 'T'
    constexpr const T* getGlobal() const noexcept
    {
      if (getGlobalID() != TypeToGlobalID<T>())
        return nullptr;
      return &getUnionMember<T>();
    }

    /// @brief Check if the Global is a function
    /// @return True if function
    constexpr bool isFn() const noexcept { return getGlobalID() == GlobalID::GLOBAL_FN; }
    /// @brief Check if the Global is a global variable
    /// @return True if variable
    constexpr bool isVar() const noexcept { return getGlobalID() == GlobalID::GLOBAL_VAR; }
    /// @brief Check if the Global is a type
    /// @return True if type
    constexpr bool isType() const noexcept { return getGlobalID() == GlobalID::GLOBAL_TYPE; }
    /// @brief Check if the Global is an alias
    /// @return True if an alias
    constexpr bool isAlias() const noexcept { return getGlobalID() == GlobalID::GLOBAL_ALIAS; }
  };
}

#undef COLTC_getUnionMember

#endif // !HG_COLT_GLOBAL
