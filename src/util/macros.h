/*****************************************************************//**
 * @file   macros.h
 * @brief  Contains helper macros.
 * 
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_MACROS
#define HG_COLT_MACROS

#include <cstdio>
#include <cstdlib>

#if defined(__has_builtin)
  #if __has_builtin(__builtin_debugtrap)
    /// @brief Intrinsic trap
    #define __DETAILS__COLT_DBREAK() __builtin_debugtrap()
  #elif __has_builtin(__debugbreak)
    /// @brief Intrinsic trap
    #define __DETAILS__COLT_DBREAK() __debugbreak()
  #endif
#endif

#ifndef __DETAILS__COLT_DBREAK
  #if defined(_MSC_VER) || defined(__INTEL_COMPILER)
    /// @brief Intrinsic trap
    #define __DETAILS__COLT_DBREAK() __debugbreak()
  #else
    /// @brief Intrinsic trap
    #define __DETAILS__COLT_DBREAK() (void)std::fgetc(stdin), std::abort()
  #endif
#endif

namespace clt
{
  [[noreturn]]
  /// @brief Aborts the program (or call the debugger if possible)
  inline void debugBreak() noexcept
  {
    __DETAILS__COLT_DBREAK();
  }
}

/// @brief Colt intrinsic debug break
#define colt_intrinsic_dbreak() clt::debugBreak()

#if defined(_MSC_VER)
  /// @brief Current function name
#define COLT_FUNCTION_NAME __FUNCSIG__
#elif defined(__clang__) || defined(__GNUC__) 
  /// @brief Current function name
#define COLT_FUNCTION_NAME __PRETTY_FUNCTION__ 
#else
  /// @brief Current function name
#define COLT_FUNCTION_NAME __func__
#endif

/// @brief The line
#define COLT_LINE_NUM __LINE__
/// @brief The current filename
#define COLT_FILENAME __FILE__

/// @brief Internal concatenating macro
#define __DETAILS__COLT_CONCAT(a, b) a##b
/// @brief Concatenates 'a' and 'b'
#define COLT_CONCAT(a, b) __DETAILS__COLT_CONCAT(a, b)

/// @brief Pair of ()
#define __DETAILS__COLT_PARENS ()

/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_EXPAND(...)  __DETAILS__COLT_EXPAND4(__DETAILS__COLT_EXPAND4(__DETAILS__COLT_EXPAND4(__DETAILS__COLT_EXPAND4(__VA_ARGS__))))
/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_EXPAND4(...) __DETAILS__COLT_EXPAND3(__DETAILS__COLT_EXPAND3(__DETAILS__COLT_EXPAND3(__DETAILS__COLT_EXPAND3(__VA_ARGS__))))
/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_EXPAND3(...) __DETAILS__COLT_EXPAND2(__DETAILS__COLT_EXPAND2(__DETAILS__COLT_EXPAND2(__DETAILS__COLT_EXPAND2(__VA_ARGS__))))
/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_EXPAND2(...) __DETAILS__COLT_EXPAND1(__DETAILS__COLT_EXPAND1(__DETAILS__COLT_EXPAND1(__DETAILS__COLT_EXPAND1(__VA_ARGS__))))
/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_EXPAND1(...) __VA_ARGS__

/// @brief Applies 'macro' on each arguments
#define COLT_FOR_EACH(macro, ...)  \
  __VA_OPT__(__DETAILS__COLT_EXPAND(__DETAILS__COLT_FOR_EACH_HELPER(macro, __VA_ARGS__)))

/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_FOR_EACH_HELPER(macro, a1, ...) \
  macro(a1) \
  __VA_OPT__(__DETAILS__COLT_FOR_EACH_AGAIN __DETAILS__COLT_PARENS (macro, __VA_ARGS__))
/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_FOR_EACH_AGAIN() __DETAILS__COLT_FOR_EACH_HELPER

/// @brief Applies 'macro' on each arguments, invoking 'macro(arg, <ARG>)'
#define COLT_FOR_EACH_1ARG(macro, arg, ...) \
  __VA_OPT__(__DETAILS__COLT_EXPAND(__DETAILS__COLT_FOR_EACH_HELPER_1ARG(macro, arg, __VA_ARGS__)))

/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_FOR_EACH_HELPER_1ARG(macro, arg, a1, ...) \
  macro(arg, a1) \
  __VA_OPT__(__DETAILS__COLT_FOR_EACH_AGAIN_1ARG __DETAILS__COLT_PARENS (macro, arg, __VA_ARGS__))

/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_FOR_EACH_AGAIN_1ARG() __DETAILS__COLT_FOR_EACH_HELPER_1ARG

/// @brief Applies 'macro' on each arguments, invoking 'macro(arg1, arg2, <ARG>)'
#define COLT_FOR_EACH_2ARG(macro, arg1, arg2, ...) \
  __VA_OPT__(__DETAILS__COLT_EXPAND(__DETAILS__COLT_FOR_EACH_HELPER_2ARG(macro, arg1, arg2, __VA_ARGS__)))

/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_FOR_EACH_HELPER_2ARG(macro, arg1, arg2, a1, ...) \
  macro(arg1, arg2, a1) \
  __VA_OPT__(__DETAILS__COLT_FOR_EACH_AGAIN_2ARG __DETAILS__COLT_PARENS (macro, arg1, arg2, __VA_ARGS__))

/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_FOR_EACH_AGAIN_2ARG() __DETAILS__COLT_FOR_EACH_HELPER_2ARG

/// @brief Applies 'macro' on each arguments, invoking 'macro(arg1, arg2, arg3, <ARG>)'
#define COLT_FOR_EACH_3ARG(macro, arg1, arg2, arg3, ...) \
  __VA_OPT__(__DETAILS__COLT_EXPAND(__DETAILS__COLT_FOR_EACH_HELPER_3ARG(macro, arg1, arg2, arg3, __VA_ARGS__)))

/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_FOR_EACH_HELPER_3ARG(macro, arg1, arg2, arg3, a1, ...) \
  macro(arg1, arg2, arg3, a1) \
  __VA_OPT__(__DETAILS__COLT_FOR_EACH_AGAIN_3ARG __DETAILS__COLT_PARENS (macro, arg1, arg2, arg3, __VA_ARGS__))

/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_FOR_EACH_AGAIN_3ARG() __DETAILS__COLT_FOR_EACH_HELPER_3ARG

/// @brief Applies 'macro' on each arguments, invoking 'macro(arg1, arg2, arg3, arg4, <ARG>)'
#define COLT_FOR_EACH_4ARG(macro, arg1, arg2, arg3, arg4, ...) \
  __VA_OPT__(__DETAILS__COLT_EXPAND(__DETAILS__COLT_FOR_EACH_HELPER_4ARG(macro, arg1, arg2, arg3, arg4, __VA_ARGS__)))

/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_FOR_EACH_HELPER_4ARG(macro, arg1, arg2, arg3, arg4, a1, ...) \
  macro(arg1, arg2, arg3, arg4, a1) \
  __VA_OPT__(__DETAILS__COLT_FOR_EACH_AGAIN_4ARG __DETAILS__COLT_PARENS (macro, arg1, arg2, arg3, arg4, __VA_ARGS__))

/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_FOR_EACH_AGAIN_4ARG() __DETAILS__COLT_FOR_EACH_HELPER_4ARG

#endif //!HG_COLT_MACRO
