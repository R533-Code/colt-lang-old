/*****************************************************************//**
 * @file   meta_reflect.h
 * @brief  Contains necessary code for reflection (on enums and more).
 * 
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_REFL
#define HG_COLT_REFL

#include <fmt/format.h>
#include <string_view>

#include "meta/meta_string_literal.h"
#include "meta/meta_traits.h"
#include "meta/meta_type_list.h"
#include "util/types.h"
#include "util/macros.h"

namespace clt::meta
{
  template<typename T>
  /// @brief Check if T is a reflectable through reflect<>
  /// @tparam T The type to check for
  struct is_reflectable
  {
    static constexpr bool value = false;
  };

  template<typename T> requires std::is_pointer_v<T>
    && (!std::is_const_v<T>) && (!std::is_volatile_v<T>)
  /// @brief Check if T is a reflectable through reflect<>
  /// @tparam T The type to check for
  struct is_reflectable<T>
  {
    static constexpr bool value = is_reflectable<std::remove_pointer_t<T>>::value;
  };

  template<typename T> requires std::is_reference_v<T>
    && (!std::is_const_v<T>) && (!std::is_volatile_v<T>)
  /// @brief Check if T is a reflectable through reflect<>
  /// @tparam T The type to check for
  struct is_reflectable<T>
  {
    static constexpr bool value = is_reflectable<std::remove_reference_t<T>>::value;
  };

  template<typename T> requires (std::is_const_v<T> || std::is_volatile_v<T>)
  /// @brief Check if T is a reflectable through reflect<>
  /// @tparam T The type to check for
  struct is_reflectable<T>
  {
    static constexpr bool value = is_reflectable<std::remove_cv_t<T>>::value;
  };

  template<typename T>
  /// @brief Short-hand for 'is_reflectable<T>::value'
  inline constexpr bool is_reflectable_v = is_reflectable<T>::value;
}

/// @brief Contains helpers for reflection
namespace clt::meta
{
  /// @brief The entity kind on which reflection is applied
  enum EntityKind
    : u8
  {
    /// @brief Enumerations
    IS_ENUM,
    /// @brief Pointers, references, and all other built-in types
    IS_BUILTIN,
    /// @brief User defined class
    IS_CLASS,
    /// @brief Unknown type
    IS_UNKNOWN
  };

  template<typename T>
  /// @brief DO NOT USE! USE reflect<>, The kind on which reflection is applied
  /// @tparam T The type to whose EntityKind to check
  struct entity_kind
  {
    static constexpr EntityKind value = IS_UNKNOWN;
  };

  template<typename T>
  /// @brief DO NOT USE! USE reflect<>, short-hand for 'entity_kind<T>::value'
  inline constexpr EntityKind entity_kind_v = entity_kind<T>::value;

  template<typename T>
  /// @brief DO NOT USE! USE reflect<>, The name of the type on which reflection is applied
  /// @tparam T The type whose name to check
  struct unqualified_name
  {
    static constexpr std::string_view value = "!UNKNOWN_TYPE!";
  };

  template<typename T>
  /// @brief DO NOT USE! USE reflect<>, short-hand for 'unqualified_name<T>::value'
  inline constexpr std::string_view unqualified_name_v = unqualified_name<T>::value;

  template<typename T>
  /// @brief DO NOT USE! USE reflect<>, The members type list of the type on which reflection is applied
  /// @tparam T The type whose type list to check
  struct members_type
  {
    using type = meta::type_list<>;
  };

  template<typename T>
  /// @brief DO NOT USE! USE reflect<>, short-hand for 'members_type<T>::type'
  using members_type_t = typename members_type<T>::type;

  template<typename On>
  /// @brief DO NOT USE! USE reflect<>, Overload for each reflected type to apply a lambda on each of its members
  struct apply_on_members
  {
    template<typename F> requires std::is_invocable_v<F, On>
    constexpr void operator()(On&&, F&&) const
      noexcept(std::is_nothrow_invocable_v<F, On>)
    {
      //Does nothing...
    }
  };
  
  template<typename On>
  /// @brief DO NOT USE! USE reflect<>, Overload for each reflected type to apply a lambda on each of its methods
  struct apply_on_methods
  {
    template<typename F> requires std::is_invocable_v<F, On>
    constexpr void operator()(On&&, F&&) const
      noexcept(std::is_nothrow_invocable_v<F, On>)
    {
      //Does nothing...
    }
  };
}

namespace clt
{
  namespace details
  {
    /// @brief Helper string for generating type name
    static constexpr std::string_view PTR_STR_START   = "PTR<";
    /// @brief Helper string for generating type name
    static constexpr std::string_view PTR_STR_END     = ">";

    /// @brief Helper string for generating type name
    static constexpr std::string_view DOUBLE_REF      = "&&";
    /// @brief Helper string for generating type name
    static constexpr std::string_view SINGLE_REF      = "&";
    /// @brief Helper string for generating type name
    static constexpr std::string_view CONST_STR       = "const ";
    /// @brief Helper string for generating type name
    static constexpr std::string_view VOLATILE_STR    = "volatile ";

  }

  template<typename T>
  /// @brief Deactivated reflection. Use COLT_DECLARE_TYPE to add reflection for a type.
  struct reflect {};

  template<typename T> requires meta::is_reflectable_v<T>
    && (!std::is_pointer_v<T>) && (!std::is_reference_v<T>)
    && (!std::is_const_v<T>) && (!std::is_volatile_v<T>)
  /// @brief Unspecialized reflection informations
  /// @tparam T The type on which to reflect
  struct reflect<T>
  {
    static constexpr meta::EntityKind kind() noexcept
    {
      return meta::entity_kind_v<T>;
    }

    static constexpr std::string_view str() noexcept
    {
      return meta::unqualified_name_v<T>;
    }

    using members_type = meta::members_type_t<T>;

    template<typename On, typename F> requires std::same_as<On, std::decay_t<T>>
    static constexpr void apply_on_members(On&& obj, F&& fn)
    {
      meta::apply_on_members<T>{}(std::forward<On>(obj), std::forward<F>(fn));
    }

    template<typename On, typename F> requires std::same_as<On, std::decay_t<T>>
    static constexpr void apply_on_methods(On&& obj, F&& fn)
    {
      meta::apply_on_methods<T>{}(std::forward<On>(obj), std::forward<F>(fn));
    }
  };
  
  template<typename T> requires std::is_pointer_v<T>
    && meta::is_reflectable_v<std::remove_pointer_t<T>>
    && (!std::is_const_v<T>) && (!std::is_volatile_v<T>)
  /// @brief Specialized reflection for pointers
  /// @tparam T The pointer on which to reflect
  struct reflect<T>
  {
  private:
    static constexpr std::string_view _NAME = reflect<std::remove_pointer_t<T>>::str();

  public:
    static constexpr meta::EntityKind kind() noexcept
    {
      return meta::IS_BUILTIN;
    }

    static constexpr std::string_view str() noexcept
    {
      return meta::join_strv_v<details::PTR_STR_START,
        _NAME,
        details::PTR_STR_END
      >;
    }

    using members_type = typename meta::members_type_t<std::remove_pointer_t<T>>::template apply<std::add_pointer>;

    template<typename On, typename F> requires std::same_as<On, std::decay_t<T>>
    static constexpr void apply_on_members(On&&, F&&) noexcept
    {
      //Pointers do not have members... Do nothing.
    }

    template<typename On, typename F> requires std::same_as<On, std::decay_t<T>>
    static constexpr void apply_on_methods(On&&, F&&) noexcept
    {
      //Pointers do not have methods... Do nothing.
    }
  };

  template<typename T> requires std::is_lvalue_reference_v<T>
    && meta::is_reflectable_v<std::remove_reference_t<T>>
    && (!std::is_const_v<T>) && (!std::is_volatile_v<T>)
  /// @brief Specialized reflection for lvalue references
  /// @tparam T The reference on which to reflect
  struct reflect<T>
  {
  private:
    static constexpr std::string_view _NAME = reflect<std::remove_reference_t<T>>::str();

  public:
    static constexpr meta::EntityKind kind() noexcept
    {
      return meta::IS_BUILTIN;
    }

    static constexpr std::string_view str() noexcept
    {
      return meta::join_strv_v<
        _NAME,
        details::SINGLE_REF
      >;
    }
    
    using members_type = typename meta::members_type_t<std::remove_reference_t<T>>::template apply<std::add_lvalue_reference>;

    template<typename On, typename F> requires std::same_as<On, std::decay_t<T>>
    static constexpr void apply_on_members(On&&, F&&) noexcept
    {
      //References do not have members... Do nothing.
    }

    template<typename On, typename F> requires std::same_as<On, std::decay_t<T>>
    static constexpr void apply_on_methods(On&&, F&&) noexcept
    {
      //References do not have members... Do nothing.
    }
  };

  template<typename T> requires std::is_rvalue_reference_v<T>
    && meta::is_reflectable_v<std::remove_reference_t<T>>
    && (!std::is_const_v<T>) && (!std::is_volatile_v<T>)
  /// @brief Specialized reflection for rvalue references
  /// @tparam T The reference on which to reflect
  struct reflect<T>
  {
  private:
    static constexpr std::string_view _NAME = reflect<std::remove_reference_t<T>>::str();

  public:
    static constexpr meta::EntityKind kind() noexcept
    {
      return meta::IS_BUILTIN;
    }

    static constexpr std::string_view str() noexcept
    {
      return meta::join_strv_v<
        _NAME,
        details::DOUBLE_REF
      >;
    }

    using members_type = typename meta::members_type_t<std::remove_reference_t<T>>::template apply<std::add_rvalue_reference>;

    template<typename On, typename F> requires std::same_as<On, std::decay_t<T>>
    static constexpr void apply_on_members(On&&, F&&) noexcept
    {
      //References do not have members... Do nothing.
    }

    template<typename On, typename F> requires std::same_as<On, std::decay_t<T>>
    static constexpr void apply_on_methods(On&&, F&&) noexcept
    {
      //References do not have members... Do nothing.
    }
  };

  template<typename T> requires meta::is_reflectable_v<std::remove_cv_t<T>>
    && std::is_const_v<T> && (!std::is_volatile_v<T>)
  /// @brief Specialized reflection for 'const' types
  /// @tparam T The type on which to reflect
  struct reflect<T>
    : public reflect<std::remove_cv_t<T>>
  {
  private:
    static constexpr std::string_view _NAME = reflect<std::remove_cv_t<T>>::str();
  
  public:
    static constexpr meta::EntityKind kind() noexcept
    {
      return meta::entity_kind_v<std::remove_cv_t<T>>;
    }

    static constexpr std::string_view str() noexcept
    {
      return meta::join_strv_v<
        details::CONST_STR,
        _NAME
      >;
    }

    using members_type = typename meta::members_type_t<std::remove_cv_t<T>>::template apply<std::add_const>;

    template<typename On, typename F> requires std::same_as<On, std::decay_t<T>>
    static constexpr void apply_on_members(On&& obj, F&& fn)
    {
      reflect<std::remove_reference_t<T>>::apply_on_members(
        std::forward<On>(obj), std::forward<F>(fn)
      );
    }

    template<typename On, typename F> requires std::same_as<On, std::decay_t<T>>
    static constexpr void apply_on_methods(On&& obj, F&& fn)
    {
      reflect<std::remove_reference_t<T>>::apply_on_methods(
        std::forward<On>(obj), std::forward<F>(fn)
      );
    }
  };

  template<typename T> requires meta::is_reflectable_v<std::remove_cv_t<T>>
    && std::is_volatile_v<T> && (!std::is_const_v<T>)
  /// @brief Specialized reflection for 'volatile' types
  /// @tparam T The type on which to reflect
  struct reflect<T>
    : public reflect<std::remove_cv_t<T>>
  {
  private:
    static constexpr std::string_view _NAME = reflect<std::remove_cv_t<T>>::str();

  public:
    static constexpr meta::EntityKind kind() noexcept
    {
      return meta::entity_kind_v<std::remove_cv_t<T>>;
    }

    static constexpr std::string_view str() noexcept
    {
      return meta::join_strv_v<
        details::VOLATILE_STR,
        _NAME
      >;
    }

    using members_type = typename meta::members_type_t<std::remove_cv_t<T>>::template apply<std::add_volatile>;

    template<typename On, typename F> requires std::same_as<On, std::decay_t<T>>
    static constexpr void apply_on_members(On&& obj, F&& fn)
    {
      reflect<std::remove_reference_t<T>>::apply_on_members(
        std::forward<On>(obj), std::forward<F>(fn)
      );
    }

    template<typename On, typename F> requires std::same_as<On, std::decay_t<T>>
    static constexpr void apply_on_methods(On&& obj, F&& fn)
    {
      reflect<std::remove_reference_t<T>>::apply_on_methods(
        std::forward<On>(obj), std::forward<F>(fn)
      );
    }
  };

  template<typename T> requires meta::is_reflectable_v<std::remove_cv_t<T>>
    && std::is_const_v<T> && std::is_volatile_v<T>
  /// @brief Specialized reflection for 'const volatile' types
  /// @tparam T The type on which to reflect
  struct reflect<T>
    : public reflect<std::remove_cv_t<T>>
  {
  private:
    static constexpr std::string_view _NAME = reflect<std::remove_cv_t<T>>::str();

  public:
    static constexpr meta::EntityKind kind() noexcept
    {
      return meta::entity_kind_v<std::remove_cv_t<T>>;
    }

    static constexpr std::string_view str() noexcept
    {
      return meta::join_strv_v<
        details::CONST_STR,
        details::VOLATILE_STR,
        _NAME
      >;
    }

    using members_type = typename meta::members_type_t<std::remove_cv_t<T>>::template apply<std::add_cv>;

    template<typename On, typename F> requires std::same_as<On, std::decay_t<T>>    
    static constexpr void apply_on_members(On&& obj, F&& fn)
    {
      reflect<std::remove_reference_t<T>>::apply_on_members(
        std::forward<On>(obj), std::forward<F>(fn)
      );
    }

    template<typename On, typename F> requires std::same_as<On, std::decay_t<T>>
    static constexpr void apply_on_methods(On&& obj, F&& fn)
    {
      reflect<std::remove_reference_t<T>>::apply_on_methods(
        std::forward<On>(obj), std::forward<F>(fn)
      );
    }
  };
  
  template <typename F, typename... Ts>
    requires std::conjunction_v<std::is_invocable<F, Ts>...>
  /// @brief Calls 'f' with each arguments
  /// @tparam F The lambda type
  /// @tparam ...Ts The parameter pack
  /// @tparam  SFINAE helper
  /// @param f The lambda to call
  /// @param ...args The arguments to pass to the lambda
  constexpr void for_each(F&& f, Ts&&... args) noexcept
  {
    (f(std::forward<Ts>(args)), ...);
  }

  namespace meta
  {
    /// @brief Tag struct for iterating over members
    struct MembersT{};
    /// @brief Tag struct for iterating over methods
    struct MethodsT{};
  }

  /// @brief Tag object for iterating over members
  inline constexpr meta::MembersT Members;

  /// @brief Tag object for iterating over methods
  inline constexpr meta::MethodsT Methods;

  template<typename T, typename F> requires meta::is_reflectable_v<T>
  /// @brief Calls 'f' with each registered member of 'of'
  /// @tparam F The lambda type
  /// @tparam T The object type
  /// @param  Helper type (usually 'Members')
  /// @param of The object whose members to access
  /// @param f The lambda function
  constexpr void for_each(meta::MembersT, T&& of, F&& f) noexcept
  {
    reflect<std::decay_t<T>>::apply_on_members(std::forward<T>(of), std::forward<F>(f));
  }

  template<typename T, typename F> requires meta::is_reflectable_v<T>
  /// @brief Calls 'f' with each registered methods of 'of'
  /// @tparam F The lambda type
  /// @tparam T The object type
  /// @param  Helper type (usually 'Methods')
  /// @param of The object whose members to access
  /// @param f The lambda function
  constexpr void for_each(meta::MethodsT, T&& of, F&& f) noexcept
  {
    reflect<std::decay_t<T>>::apply_on_methods(std::forward<T>(of), std::forward<F>(f));
  }
}

/// @brief Declares reflection informations for built-in 'TYPE'
#define DECLARE_BUILTIN_TYPE(TYPE) \
template<> \
struct clt::meta::entity_kind<TYPE> \
{ \
  static constexpr clt::meta::EntityKind value = clt::meta::IS_BUILTIN; \
}; \
template<> \
struct clt::meta::is_reflectable<TYPE> \
{ \
  static constexpr bool value = true; \
}; \
\
template<> \
struct clt::meta::members_type<TYPE> \
{ \
  using type = meta::type_list<>; \
}; \
\
template<> \
struct clt::meta::unqualified_name<TYPE> \
{ \
  static constexpr std::string_view value = #TYPE; \
}

/***************************
* Declare all built-in types
****************************/

DECLARE_BUILTIN_TYPE(char);
DECLARE_BUILTIN_TYPE(i8);
DECLARE_BUILTIN_TYPE(u8);
DECLARE_BUILTIN_TYPE(i16);
DECLARE_BUILTIN_TYPE(u16);
DECLARE_BUILTIN_TYPE(i32);
DECLARE_BUILTIN_TYPE(u32);
DECLARE_BUILTIN_TYPE(i64);
DECLARE_BUILTIN_TYPE(u64);
DECLARE_BUILTIN_TYPE(f32);
DECLARE_BUILTIN_TYPE(f64);

//We no longer need this macro
#undef DECLARE_BUILTIN_TYPE

#define COLT_DETAILS_MEMBER_TO_MEMBER_PTR(type, name) , decltype(&type::name)
#define COLT_DETAILS_IF_CONSTEXPR_IS_MEMBER_CALL_LAMBDA(TYPE, member) if constexpr (std::is_member_object_pointer_v<decltype(&TYPE::member)>) fn(obj.member);
#define COLT_DETAILS_IF_CONSTEXPR_IS_METHOD_CALL_LAMBDA(TYPE, member) if constexpr (std::is_member_function_pointer_v<decltype(&TYPE::member)>) fn(&obj.member);

/// @brief Adds the necessary friends declarations for reflection
#define COLT_ENABLE_REFLECTION() template<typename T> friend struct clt::metaect

/// @brief Enables reflection on a type.
/// @code{.cpp}
/// class Test
/// {
///   u64 a;
///   u64 b;
/// 
///   //Required for private members
///   COLT_ENABLE_REFLECTION();
/// };
/// COLT_DECLARE_TYPE(Test, a, b);
/// @endcode
#define COLT_DECLARE_TYPE(TYPE, member, ...) \
  template<> \
  struct clt::meta::entity_kind<TYPE> \
  { \
    static constexpr clt::meta::EntityKind value = clt::meta::IS_CLASS; \
  }; \
  template<> \
  struct clt::meta::is_reflectable<TYPE> \
  { \
    static constexpr bool value = true; \
  }; \
  template<> \
  struct clt::metaect<TYPE> { \
    static constexpr clt::meta::EntityKind kind() noexcept { return clt::meta::IS_CLASS; } \
    static constexpr std::string_view str() noexcept { return #TYPE; } \
    using members_type = typename clt::meta::type_list<decltype(&TYPE::member) COLT_FOR_EACH_1ARG(COLT_DETAILS_MEMBER_TO_MEMBER_PTR, TYPE, __VA_ARGS__)> \
      ::template remove_if<std::is_member_function_pointer>; \
    using methods_type = typename clt::meta::type_list<decltype(&TYPE::member) COLT_FOR_EACH_1ARG(COLT_DETAILS_MEMBER_TO_MEMBER_PTR, TYPE, __VA_ARGS__)> \
      ::template remove_if<std::is_member_object_pointer>; \
    static constexpr size_t members_count() noexcept { return members_type::size; } \
    static constexpr size_t methods_count() noexcept { return members_type::size; } \
    template<typename On, typename F> requires std::same_as<std::decay_t<On>, std::decay_t<TYPE>>\
    static constexpr void apply_on_members(On&& obj, F&& fn) { \
        if constexpr (std::is_member_object_pointer_v<decltype(&TYPE::member)>) \
          fn(obj.member); \
        COLT_FOR_EACH_1ARG(COLT_DETAILS_IF_CONSTEXPR_IS_MEMBER_CALL_LAMBDA, TYPE, __VA_ARGS__) \
    } \
    template<typename On, typename F> requires std::same_as<std::decay_t<On>, std::decay_t<TYPE>>\
    static constexpr void apply_on_methods(On&& obj, F&& fn) { \
        if constexpr (std::is_member_function_pointer_v<decltype(&TYPE::member)>) \
          fn(&obj.member); \
        COLT_FOR_EACH_1ARG(COLT_DETAILS_IF_CONSTEXPR_IS_METHOD_CALL_LAMBDA, TYPE, __VA_ARGS__) \
    } \
  }

template<typename T>
  requires clt::meta::is_reflectable_v<T>
  && (clt::reflect<T>::kind() == clt::meta::IS_CLASS) && (!fmt::is_formattable<T>::value)
struct fmt::formatter<T>
  : public clt::meta::DefaultParserFMT
{
  template<typename FormatContext>
  auto format(const T& obj, FormatContext& ctx)
  {
    using namespace clt;   
    auto it = fmt::format_to(ctx.out(), "{}: {{ ", reflect<T>::str());
    
    u64 i = 0;
    for_each(Members, obj, [&]<typename Ti>(const Ti& o) {
      it = fmt::format_to(it, "{}{}", o, ++i == reflect<T>::members_count() ? " }" : ", ");
    });
    return it;
  }
};

#endif //!HG_COLT_REFL