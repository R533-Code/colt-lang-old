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
    size_t size = 0; \
    using ty = enum_name; \
    COLT_FOR_EACH(IMPL_CONVERT_TYPE_TO_ENUM, type_list) \
  }

/// @brief Expands to default copy/move constructor/assignment operator
#define MAKE_DEFAULT_COPY_AND_MOVE_FOR(type) \
  constexpr type(type&&) noexcept = default; \
  constexpr type(const type&) noexcept = default; \
  constexpr type& operator=(type&&) noexcept = default; \
  constexpr type& operator=(const type&) noexcept = default;

#endif // !HG_COLTC_MACRO_HELPER
