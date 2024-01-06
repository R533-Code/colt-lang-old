/** @file assert_true.h
* Contains assertions helpers that work also work at compile-time.
*/

#ifndef HG_COLT_ASSERT_TRUE
#define HG_COLT_ASSERT_TRUE

#include <type_traits>
#include <source_location>
#include "io/print.h"
#include "macros.h"
#include "config_type.h"

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

  template<typename... BoolTs> requires (sizeof...(BoolTs) != 0)
    /// @brief Asserts that multiple conditions are true, and if not,
    /// stops the application and prints the failed assertions.
    /// @tparam ...BoolTs The type parameter pack
    /// @param message The message to print
    /// @param src The source location
    /// @param ...bools The Assertion pack
    constexpr void assert_true_multiple(const char* message, std::source_location src, BoolTs... bools) noexcept
  {
    static_assert((std::is_same_v<Assertion, std::remove_cvref_t<BoolTs>> && ...),
      "This function expects 'Assertion'! Use assert_true rather than calling it directly!");
    if (std::is_constant_evaluated())
    {
      Assertion* array[sizeof...(BoolTs)] = { &bools... };
      for (size_t i = 0; i < sizeof...(BoolTs); i++)
      {
        if (array[i]->value)
          continue;
        constexpr_assert_true_failed();
      }
    }
    else if constexpr (isDebugBuild())
    {
      Assertion* array[sizeof...(BoolTs)] = { &bools... };

      bool error = false;
      for (size_t i = 0; i < sizeof...(BoolTs); i++)
      {
        if (array[i]->value)
          continue;
        if (!error)
        {
          io::print_fatal("Assertion failed in function '{}' (line {}) in file:\n'{}'\n{}",
            src.function_name(), src.line(), src.file_name(), message);
          error = true;
        }
        io::print("{}) {} == false", i + 1, array[i]->str);
      }
      if (error)
        colt_intrinsic_dbreak();
    }
  }
}

namespace clt
{
  [[noreturn]]
  inline void unreachable(const char* error, std::source_location src = std::source_location::current())
  {
    if constexpr (isDebugBuild())
    {
      io::print_fatal("Unreachable branch hit in function '{}' (line {}) in file:\n'{}'\n{}",
        src.function_name(), src.line(), src.file_name(), error);
    }
    debugBreak();
  }
}

/// @brief Helper for transforming assertions into strings and their evaluated value
#define __DETAILS__COLT_TO_ASSERTION(expr) , clt::details::Assertion{ #expr, (expr) }

/// @brief Asserts that all condition are true
#define assert_true(MESSAGE, COND, ...) clt::details::assert_true_multiple(MESSAGE, std::source_location::current() __DETAILS__COLT_TO_ASSERTION(COND) COLT_FOR_EACH(__DETAILS__COLT_TO_ASSERTION, __VA_ARGS__))

/// @brief switch case with no default
#define switch_no_default(...) switch (__VA_ARGS__) \
  default: \
    if (true) { clt::unreachable("Invalid value for 'switch_no_default'."); } \
    else

#endif //!HG_COLT_ASSERT_TRUE