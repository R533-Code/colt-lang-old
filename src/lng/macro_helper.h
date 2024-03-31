/*****************************************************************//**
 * @file   macro_helper.h
 * @brief  Contains ugly macro hacks to simplify code in
 *         `colt_global`, `colt_type`, `colt_expr`.
 * 
 * @author RPC
 * @date   March 2024
 *********************************************************************/
#ifndef HG_COLTC_MACRO_HELPER
#define HG_COLTC_MACRO_HELPER

#include "util/macros.h"
#include "util/token_helper.h"

#define IMPL_FORWARD_DECLARE_TYPE_LIST(a) class a;
#define IMPL_CONVERT_TYPE_TO_ENUM(a) if (std::same_as<T, a>) return static_cast<ty>(size); else ++size;

/// @brief Forward declares a list of types, see `colt_type.h`
#define FORWARD_DECLARE_TYPE_LIST(macro) COLT_FOR_EACH(IMPL_FORWARD_DECLARE_TYPE_LIST, macro)

/// @brief Creates a function named TypeTo##enum_name, that takes a type
///         as template parameter and converts it to its enum equivalent,
///         see `colt_type.h`.
/// This macro relies on the fact that the 'type_list' contains
/// the types in the same order than the enum they represent.
#define CONVERT_TYPES_TO_ENUM(enum_name, type_list) template<typename T> \
  consteval enum_name TypeTo##enum_name() noexcept {\
    /*static_assert(clt::reflect<enum_name>::count() == meta::type_list<type_list>::size, "Check the sizes of arguments!");*/ \
    size_t size = 0; \
    using ty = enum_name; \
    COLT_FOR_EACH(IMPL_CONVERT_TYPE_TO_ENUM, type_list) \
  }

#define IMPL_MAKE_UNION(a) a _##a;
#define IMPL_GET_MEMBER(a) if constexpr (std::same_as<T, a>) return _##a;

/// @brief Helper used in variants, see `colt_global.h.
/// Creates a union containing the types in the type list, with member
/// name _ followed by the type name.
#define MAKE_UNION_AND_GET_MEMBER(type_list) \
  template<typename T, typename... Args> \
  static constexpr std::monostate construct(T* placement, Args&&... args) \
  { \
    std::construct_at(placement, std::forward<Args>(args)...); \
    return {}; \
  } \
  union { \
    std::monostate _mono_state_; \
    COLT_FOR_EACH(IMPL_MAKE_UNION, type_list) \
  }; \
  template<typename T> \
  constexpr const auto& getUnionMember() const noexcept { COLT_FOR_EACH(IMPL_GET_MEMBER, type_list) }  \
  template<typename T> \
  constexpr auto& getUnionMember() noexcept { COLT_FOR_EACH(IMPL_GET_MEMBER, type_list) }



#endif // !HG_COLTC_MACRO_HELPER
