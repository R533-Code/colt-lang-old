/*****************************************************************/ /**
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
#include <concepts>
#include <source_location>

#include "io/print.h"
#include "colt_config.h"

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

#if defined(COLT_MSVC)
  #define CLT_EXPORT __declspec(dllexport)
  #define CLT_IMPORT __declspec(dllimport)
#elif defined(COLT_GNU) || defined(COLT_CLANG)
  #define CLT_EXPORT __attribute__((visibility("default")))
  #define CLT_IMPORT
#else
  #define CLT_EXPORT
  #define CLT_IMPORT
  #pragma warning "Unknown dynamic link import/export semantics."
#endif

namespace clt
{
  [[noreturn]]
  /// @brief Aborts the program (or call the debugger if possible)
  inline void
      debug_break() noexcept
  {
    __DETAILS__COLT_DBREAK();
  }
} // namespace clt

/// @brief Colt intrinsic debug break
#define colt_intrinsic_dbreak() clt::debug_break()

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
#define __DETAILS__COLT_EXPAND(...)                \
  __DETAILS__COLT_EXPAND4(__DETAILS__COLT_EXPAND4( \
      __DETAILS__COLT_EXPAND4(__DETAILS__COLT_EXPAND4(__VA_ARGS__))))
/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_EXPAND4(...)               \
  __DETAILS__COLT_EXPAND3(__DETAILS__COLT_EXPAND3( \
      __DETAILS__COLT_EXPAND3(__DETAILS__COLT_EXPAND3(__VA_ARGS__))))
/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_EXPAND3(...)               \
  __DETAILS__COLT_EXPAND2(__DETAILS__COLT_EXPAND2( \
      __DETAILS__COLT_EXPAND2(__DETAILS__COLT_EXPAND2(__VA_ARGS__))))
/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_EXPAND2(...)               \
  __DETAILS__COLT_EXPAND1(__DETAILS__COLT_EXPAND1( \
      __DETAILS__COLT_EXPAND1(__DETAILS__COLT_EXPAND1(__VA_ARGS__))))
/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_EXPAND1(...) __VA_ARGS__

/// @brief Applies 'macro' on each arguments
#define COLT_FOR_EACH(macro, ...) \
  __VA_OPT__(                     \
      __DETAILS__COLT_EXPAND(__DETAILS__COLT_FOR_EACH_HELPER(macro, __VA_ARGS__)))

/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_FOR_EACH_HELPER(macro, a1, ...) \
  macro(a1) __VA_OPT__(                                 \
      __DETAILS__COLT_FOR_EACH_AGAIN __DETAILS__COLT_PARENS(macro, __VA_ARGS__))
/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_FOR_EACH_AGAIN() __DETAILS__COLT_FOR_EACH_HELPER

/// @brief Applies 'macro' on each arguments, invoking 'macro(arg, <ARG>)'
#define COLT_FOR_EACH_1ARG(macro, arg, ...) \
  __VA_OPT__(__DETAILS__COLT_EXPAND(        \
      __DETAILS__COLT_FOR_EACH_HELPER_1ARG(macro, arg, __VA_ARGS__)))

/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_FOR_EACH_HELPER_1ARG(macro, arg, a1, ...)            \
  macro(arg, a1)                                                             \
      __VA_OPT__(__DETAILS__COLT_FOR_EACH_AGAIN_1ARG __DETAILS__COLT_PARENS( \
          macro, arg, __VA_ARGS__))

/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_FOR_EACH_AGAIN_1ARG() __DETAILS__COLT_FOR_EACH_HELPER_1ARG

/// @brief Applies 'macro' on each arguments, invoking 'macro(arg1, arg2, <ARG>)'
#define COLT_FOR_EACH_2ARG(macro, arg1, arg2, ...) \
  __VA_OPT__(__DETAILS__COLT_EXPAND(               \
      __DETAILS__COLT_FOR_EACH_HELPER_2ARG(macro, arg1, arg2, __VA_ARGS__)))

/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_FOR_EACH_HELPER_2ARG(macro, arg1, arg2, a1, ...)     \
  macro(arg1, arg2, a1)                                                      \
      __VA_OPT__(__DETAILS__COLT_FOR_EACH_AGAIN_2ARG __DETAILS__COLT_PARENS( \
          macro, arg1, arg2, __VA_ARGS__))

/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_FOR_EACH_AGAIN_2ARG() __DETAILS__COLT_FOR_EACH_HELPER_2ARG

/// @brief Applies 'macro' on each arguments, invoking 'macro(arg1, arg2, arg3, <ARG>)'
#define COLT_FOR_EACH_3ARG(macro, arg1, arg2, arg3, ...) \
  __VA_OPT__(__DETAILS__COLT_EXPAND(                     \
      __DETAILS__COLT_FOR_EACH_HELPER_3ARG(macro, arg1, arg2, arg3, __VA_ARGS__)))

/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_FOR_EACH_HELPER_3ARG(macro, arg1, arg2, arg3, a1, ...) \
  macro(arg1, arg2, arg3, a1)                                                  \
      __VA_OPT__(__DETAILS__COLT_FOR_EACH_AGAIN_3ARG __DETAILS__COLT_PARENS(   \
          macro, arg1, arg2, arg3, __VA_ARGS__))

/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_FOR_EACH_AGAIN_3ARG() __DETAILS__COLT_FOR_EACH_HELPER_3ARG

/// @brief Applies 'macro' on each arguments, invoking 'macro(arg1, arg2, arg3, arg4, <ARG>)'
#define COLT_FOR_EACH_4ARG(macro, arg1, arg2, arg3, arg4, ...)            \
  __VA_OPT__(__DETAILS__COLT_EXPAND(__DETAILS__COLT_FOR_EACH_HELPER_4ARG( \
      macro, arg1, arg2, arg3, arg4, __VA_ARGS__)))

/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_FOR_EACH_HELPER_4ARG(                                \
    macro, arg1, arg2, arg3, arg4, a1, ...)                                  \
  macro(arg1, arg2, arg3, arg4, a1)                                          \
      __VA_OPT__(__DETAILS__COLT_FOR_EACH_AGAIN_4ARG __DETAILS__COLT_PARENS( \
          macro, arg1, arg2, arg3, arg4, __VA_ARGS__))

/// @brief Helper for COLT_FOR_EACH_*
#define __DETAILS__COLT_FOR_EACH_AGAIN_4ARG() __DETAILS__COLT_FOR_EACH_HELPER_4ARG

#define IMPL_FORWARD_DECLARE_TYPE_LIST(a) class a;
#define IMPL_CONVERT_TYPE_TO_ENUM(a) \
  if (std::same_as<T, a>)            \
    return static_cast<ty>(size);    \
  else                               \
    ++size;

/// @brief Forward declares a list of types, see `colt_type.h`
#define FORWARD_DECLARE_TYPE_LIST(macro) \
  COLT_FOR_EACH(IMPL_FORWARD_DECLARE_TYPE_LIST, macro)

/// @brief Creates a function named TypeTo##enum_name, that takes a type
///         as template parameter and converts it to its enum equivalent,
///         see `colt_type.h`.
/// This macro relies on the fact that the 'type_list' contains
/// the types in the same order than the enum they represent.
#define CONVERT_TYPES_TO_ENUM(enum_name, type_list)                                                                           \
  template<typename T>                                                                                                        \
  consteval enum_name TypeTo##enum_name() noexcept                                                                            \
  {                                                                                                                           \
    /*static_assert(clt::reflect<enum_name>::count() == meta::type_list<type_list>::size, "Check the sizes of arguments!");*/ \
    size_t size = 0;                                                                                                          \
    using ty    = enum_name;                                                                                                  \
    COLT_FOR_EACH(IMPL_CONVERT_TYPE_TO_ENUM, type_list)                                                                       \
  }

#define IMPL_MAKE_UNION(a) a _##a;
#define IMPL_GET_SIZEOF(a) , sizeof(a)
#define IMPL_GET_MEMBER(a)          \
  if constexpr (std::same_as<T, a>) \
    return _##a;

/// @brief Helper used in variants, see `colt_global.h.
/// Creates a union containing the types in the type list, with member
/// name _ followed by the type name.
#define MAKE_UNION_AND_GET_MEMBER(type_list)                                       \
  template<typename T, typename... Args>                                           \
  static constexpr void construct(T* placement, Args&&... args)                    \
  {                                                                                \
    std::construct_at(placement, std::forward<Args>(args)...);                     \
  }                                                                                \
  union                                                                            \
  {                                                                                \
    char _buffer[std::max({(size_t)0 COLT_FOR_EACH(IMPL_GET_SIZEOF, type_list)})]; \
    COLT_FOR_EACH(IMPL_MAKE_UNION, type_list)                                      \
  };                                                                               \
  template<typename T>                                                             \
  constexpr const auto& getUnionMember() const noexcept                            \
  {                                                                                \
    COLT_FOR_EACH(IMPL_GET_MEMBER, type_list)                                      \
  }                                                                                \
  template<typename T>                                                             \
  constexpr auto& getUnionMember() noexcept                                        \
  {                                                                                \
    COLT_FOR_EACH(IMPL_GET_MEMBER, type_list)                                      \
  }

/// @brief Expands to default copy/move constructor/assignment operator
#define MAKE_DEFAULT_COPY_AND_MOVE_FOR(type)                 \
  constexpr type(type&&) noexcept                 = default; \
  constexpr type(const type&) noexcept            = default; \
  constexpr type& operator=(type&&) noexcept      = default; \
  constexpr type& operator=(const type&) noexcept = default;

/// @brief Expands to delete copy/move constructor/assignment operator
#define MAKE_DELETE_COPY_AND_MOVE_FOR(type) \
  type(type&&)                 = delete;    \
  type(const type&)            = delete;    \
  type& operator=(type&&)      = delete;    \
  type& operator=(const type&) = delete;

#define IMPL_CREATE_TOKEN_FRIEND_CLASS(a) friend class a;

/// @brief Creates a token class
#define CREATE_TOKEN_TYPE(name, type, max_value, friend1, ...)          \
  struct name                                                           \
  {                                                                     \
    type index;                                                         \
    constexpr name(type index) noexcept                                 \
        : index(index)                                                  \
    {                                                                   \
    }                                                                   \
                                                                        \
  public:                                                               \
    constexpr type getID() const noexcept                               \
    {                                                                   \
      return index;                                                     \
    }                                                                   \
    MAKE_DEFAULT_COPY_AND_MOVE_FOR(name);                               \
    COLT_FOR_EACH(IMPL_CREATE_TOKEN_FRIEND_CLASS, friend1, __VA_ARGS__) \
    constexpr bool operator==(const name&) const noexcept = default;    \
    using storage_t                                       = type;       \
    static constexpr type MAX_VALUE                       = max_value;  \
    template<TokenType T>                                               \
    friend class OptTok;                                                \
  };

namespace clt::details
{
  /// @brief Responsible of storing an expressions value and its source code string
  struct Assertion
  {
    /// @brief The string representing the value
    const char* str;
    /// @brief The value of the assertion
    bool value;
  };

  /// @brief Used to cause a compile-time failure
  inline void constexpr_assert_true_failed() noexcept
  {
    //ASSERTION FAILED AT COMPILE TIME!
  }

  template<typename... BoolTs>
    requires(sizeof...(BoolTs) != 0)
  /// @brief Asserts that multiple conditions are true, and if not,
  /// stops the application and prints the failed assertions.
  /// @tparam ...BoolTs The type parameter pack
  /// @param message The message to print
  /// @param src The source location
  /// @param ...bools The Assertion pack
  constexpr void assert_true_multiple(
      const char* message, std::source_location src, BoolTs... bools) noexcept
  {
    static_assert(
        (std::is_same_v<Assertion, std::remove_cvref_t<BoolTs>> && ...),
        "This function expects 'Assertion'! Use assert_true rather than calling it "
        "directly!");
    if (std::is_constant_evaluated())
    {
      Assertion* array[sizeof...(BoolTs)] = {&bools...};
      for (size_t i = 0; i < sizeof...(BoolTs); i++)
      {
        if (array[i]->value)
          continue;
        constexpr_assert_true_failed();
      }
    }
    else if constexpr (is_debug_build())
    {
      Assertion* array[sizeof...(BoolTs)] = {&bools...};

      bool error = false;
      for (size_t i = 0; i < sizeof...(BoolTs); i++)
      {
        if (array[i]->value)
          continue;
        if (!error)
        {
          io::print_fatal(
              "Assertion failed in function '{}' (line {}) in file:\n'{}'\n{}",
              src.function_name(), src.line(), src.file_name(), message);
          error = true;
        }
        io::print("{}) {} == false", i + 1, array[i]->str);
      }
      if (error)
        colt_intrinsic_dbreak();
    }
  }
} // namespace clt::details

namespace clt
{
  [[noreturn]]
  /// @brief Marks a branch as unreachable, printing an error on Debug build
  /// @param error The error message
  /// @param src The source code information
  inline void
      unreachable(
          const char* error,
          std::source_location src = std::source_location::current())
  {
    if constexpr (is_debug_build())
    {
      io::print_fatal(
          "Unreachable branch hit in function '{}' (line {}) in file:\n'{}'\n{}",
          src.function_name(), src.line(), src.file_name(), error);
    }
    debug_break();
  }
} // namespace clt

/// @brief Helper for transforming assertions into strings and their evaluated value
#define __DETAILS__COLT_TO_ASSERTION(expr) \
  , clt::details::Assertion                \
  {                                        \
    #expr, (expr)                          \
  }

/// @brief Asserts that all condition are true
#define assert_true(MESSAGE, COND, ...)                                           \
  clt::details::assert_true_multiple(                                             \
      MESSAGE, std::source_location::current() __DETAILS__COLT_TO_ASSERTION(COND) \
                   COLT_FOR_EACH(__DETAILS__COLT_TO_ASSERTION, __VA_ARGS__))

/// @brief switch case with no default
#define switch_no_default(...)                                    \
  switch (__VA_ARGS__)                                            \
  default:                                                        \
    if (true)                                                     \
    {                                                             \
      clt::unreachable("Invalid value for 'switch_no_default'."); \
    }                                                             \
    else

namespace clt
{
  namespace details
  {
    /// @brief Tag type for constructing an empty Option
    struct NoneT
    {
    };
  } // namespace details

  /// @brief Tag object for constructing an empty Option
  static constexpr details::NoneT None;
}

namespace clt::lng
{
  template<typename T>
  /// @brief Check if a type is a token (usually created through macro CREATE_TOKEN_TYPE).
  concept TokenType = std::same_as<
                          std::remove_cv_t<typename T::storage_t>,
                          std::remove_cv_t<decltype(T::MAX_VALUE)>>
                      && std::same_as<
                          std::remove_cv_t<typename T::storage_t>,
                          std::remove_cv_t<decltype(std::declval<T>().getID())>>;

  template<TokenType T>
  /// @brief Represents an optional token
  class OptTok
  {
    static_assert(
        T::MAX_VALUE < std::numeric_limits<typename T::storage_t>::max(),
        "Use Option<> as OptTok cannot take advantage of stored data!");
    /// @brief The storage type
    using ty = typename T::storage_t;

    /// @brief The index
    ty index;

    /// @brief The invalid value used to represent None
    static constexpr ty INVALID = T::MAX_VALUE + 1;

  public:
    OptTok() = delete;

    /// @brief Constructor
    /// @param tkn The token whose content to store
    constexpr OptTok(T tkn) noexcept
        : index(tkn.getID())
    {
    }
    /// @brief Constructor
    constexpr OptTok(decltype(None)) noexcept
        : index(INVALID)
    {
    }

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(OptTok);

    /// @brief Assigns a value to the optional
    /// @param t The value to assign
    /// @return this
    constexpr OptTok& operator=(T t) noexcept
    {
      index = t.getID();
      return *this;
    }
    /// @brief Clears the optional
    /// @param  None
    /// @return this
    constexpr OptTok& operator=(decltype(None)) noexcept
    {
      index = INVALID;
      return *this;
    }

    /// @brief Check if the optional does not contain a value
    /// @return True if the optional is empty
    constexpr bool is_none() const noexcept { return index == INVALID; }
    /// @brief Check if the optional contains a value
    /// @return True if the optional is not empty
    constexpr bool is_value() const noexcept { return !is_none(); }

    /// @brief Returns the value, is_value must be true.
    /// @return The value
    constexpr T value() const noexcept
    {
      assert_true("OptTok was empty!", is_value());
      return T{index};
    }
  };
} // namespace clt::lng

#endif //!HG_COLT_MACRO
