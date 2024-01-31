/*****************************************************************//**
 * @file   expect.h
 * @brief  Contains the Expect template.
 * 
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_EXPECT
#define HG_COLT_EXPECT

#include "common.h"

namespace clt
{
  namespace meta
  {
    /// @brief Tag struct for constructing errors in Expect
    struct ErrorT{};
  }

  /// @brief Tag object for constructing errors in Expect
  inline constexpr meta::ErrorT Error;

  template<typename ExpectedTy, typename ErrorTy>
  /// @brief A helper class that can hold either a valid value or an error.
  /// This class can be seen as an Option, that carries error informations.
  /// To document source code more, to constructs an Expect containing
  /// an error, clt::Error is passed to the constructor.
  /// The default constructor of an Expect default constructs an expected value:
  /// While this might seen counterintuitive, this class is named Expect, as it
  /// will most likely contain the expected value, not an error.
  ///
  /// Example Usage:
  /// @code{.cpp}
  /// Expect<int, const char*> div(int a, int b)
  /// {
  ///   if (b != 0)
  ///     return a / b;
  ///   return { Error, "Division by zero is prohibited!" };
  /// }
  /// @endcde
  /// @tparam ExpectedTy The expected type
  /// @tparam ErrorTy The error type
  class Expect
  {
    /// @brief Buffer for both error type and expected value
    union
    {
      /// @brief The expected value (active when is_error_v == false)
      ExpectedTy expected;
      /// @brief The error value (active when is_error_v == true)
      ErrorTy error_v;
    };

    /// @brief True if an error is stored in the Expect
    bool is_error_v;

  public:
    /// @brief Default constructs an error in the Expect
    /// @param  ErrorT tag
    constexpr Expect(meta::ErrorT)
      noexcept(std::is_nothrow_constructible_v<ErrorTy>)
      : is_error_v(true)
    {
      new(&error_v) ErrorTy();
    }

    /// @brief Copy constructs an error in the Expect
    /// @param  ErrorT tag
    /// @param value The value to copy
    constexpr Expect(meta::ErrorT, const ErrorTy& value)
      noexcept(std::is_nothrow_copy_constructible_v<ErrorTy>)
      : is_error_v(true)
    {
      new(&error_v) ErrorTy(value);
    }


    /// @brief Move constructs an error in the Expect
    /// @param  ErrorT tag
    /// @param to_move The value to move
    constexpr Expect(meta::ErrorT, ErrorTy&& to_move)
      noexcept(std::is_nothrow_move_constructible_v<ErrorTy>)
      : is_error_v(true)
    {
      new(&error_v) ErrorTy(std::move(to_move));
    }

    template<typename... Args>
    /// @brief Constructs an error in place in the Expect
    /// @tparam ...Args Parameter pack
    /// @param  InPlaceT tag
    /// @param  ErrorT tag
    /// @param ...args Argument pack forwarded to the constructor
    constexpr Expect(meta::InPlaceT, meta::ErrorT, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<ErrorTy, Args...>)
      : is_error_v(true)
    {
      new(&error_v) ErrorTy(std::forward<Args>(args)...);
    }

    /// @brief Default constructs an expected value in the Expect
    constexpr Expect()
      noexcept(std::is_default_constructible_v<ExpectedTy>)
      : is_error_v(false)
    {
      new(&expected) ExpectedTy();
    }

    /// @brief Copy constructs an expected value in the Expect
    /// @param value The value to copy
    constexpr Expect(const ExpectedTy& value)
      noexcept(std::is_nothrow_copy_constructible_v<ExpectedTy>)
      : is_error_v(false)
    {
      new(&expected) ExpectedTy(value);
    }

    /// @brief Move constructs an expected value in the Expect
    /// @param to_move The value to move
    constexpr Expect(ExpectedTy&& to_move)
      noexcept(std::is_nothrow_move_constructible_v<ExpectedTy>)
    : is_error_v(false)
    {
      new(&expected) ExpectedTy(std::move(to_move));
    }

    template<typename... Args, typename Ty>
      requires (!std::is_same_v<Ty, meta::ErrorT>)
    /// @brief Constructs an expected value in place in the Expect
    /// @tparam ...Args Parameter pack
    /// @param  InPlaceT tag
    /// @param  ErrorT tag
    /// @param ...args Argument pack forwarded to the constructor
    constexpr Expect(meta::InPlaceT, Ty&& arg, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<ExpectedTy, Ty, Args...>)
      : is_error_v(false)
    {
      new(&expected) ExpectedTy(std::forward<Ty>(arg), std::forward<Args>(args)...);
    }

    /// @brief Copy constructs an Expect
    /// @param copy The Expect to copy
    constexpr Expect(const Expect& copy)
      noexcept(std::is_nothrow_copy_constructible_v<ExpectedTy>
        && std::is_nothrow_copy_constructible_v<ErrorTy>)
      : is_error_v(copy.is_error_v)
    {
      if (is_error_v)
        new(&error_v) ErrorTy(copy.error_v);
      else
        new(&expected) ExpectedTy(copy.expected);
    }

    /// @brief Copy assignment operator
    /// @param copy The Expect to copy
    /// @return Self
    constexpr Expect& operator=(const Expect& copy)
      noexcept(std::is_nothrow_copy_constructible_v<ExpectedTy>
        && std::is_nothrow_copy_constructible_v<ErrorTy>
        && std::is_nothrow_destructible_v<ErrorTy>
        && std::is_nothrow_destructible_v<ExpectedTy>)
    {
      assert_true("Self-assignment is prohibited!", &copy != this);

      if (is_error_v)
        error_v.~ErrorTy();
      else
        expected.~ExpectedTy();

      is_error_v = copy.is_error_v;
      if (is_error_v)
        new(&error_v) ErrorTy(copy.error_v);
      else
        new(&expected) ExpectedTy(copy.expected);

      return *this;
    }

    /// @brief Move constructs an Expect
    /// @param move The Expect to move
    constexpr Expect(Expect&& move)
      noexcept(std::is_nothrow_move_constructible_v<ExpectedTy>
        && std::is_nothrow_move_constructible_v<ErrorTy>)
      : is_error_v(move.is_error_v)
    {
      if (is_error_v)
        new(&error_v) ErrorTy(std::move(move.error_v));
      else
        new(&expected) ExpectedTy(std::move(move.expected));
    }

    /// @brief Move assignment operator
    /// @param move The Expect to move
    /// @return Self
    constexpr Expect& operator=(Expect&& move)
      noexcept(std::is_nothrow_move_constructible_v<ExpectedTy>
        && std::is_nothrow_move_constructible_v<ErrorTy>
        && std::is_nothrow_destructible_v<ErrorTy>
        && std::is_nothrow_destructible_v<ExpectedTy>)
    {
      assert_true("Self-assignment is prohibited!", &move != this);

      if (is_error_v)
        error_v.~ErrorTy();
      else
        expected.~ExpectedTy();

      is_error_v = move.is_error_v;
      if (is_error_v)
        new(&error_v) ErrorTy(std::move(move.error_v));
      else
        new(&expected) ExpectedTy(std::move(move.expected));

      return *this;
    }

    /// @brief Destructs the value/error contained in the Expect
    constexpr ~Expect() noexcept(std::is_nothrow_destructible_v<ExpectedTy>
      && std::is_nothrow_destructible_v<ErrorTy>)
    {
      if (is_error_v)
        error_v.~ErrorTy();
      else
        expected.~ExpectedTy();
    }

    /// @brief Check if the Expect contains an error
    /// @return True if the Expect contains an error
    constexpr bool is_error() const noexcept { return is_error_v; }
    /// @brief Check if the Expect contains an expected value
    /// @return True if the Expect contains an expected value
    constexpr bool is_expect() const noexcept { return !is_error_v; }

    /// @brief Check if the Expect contains an error.
    /// Same as is_error().
    /// @return True if the Expect contains an error
    constexpr bool operator!() const noexcept { return is_error_v; }
    /// @brief Check if the Expect contains an expected value.
    /// Same as is_expected().
    /// @return True if the Expect contains an expected value
    explicit constexpr operator bool() const noexcept { return !is_error_v; }

    /// @brief Returns the stored Expect value.
    /// @return The Expect value
    constexpr const ExpectedTy* operator->() const noexcept
    {
      assert_true("Expect contained an error!", is_expect());
      return &expected;
    }
    
    /// @brief Returns the stored Expect value.
    /// @return The Expect value
    constexpr ExpectedTy* operator->() noexcept
    {
      assert_true("Expect contained an error!", is_expect());
      return &expected;
    }

    /// @brief Returns the stored Expect value.
    /// @return The Expect value.
    constexpr const ExpectedTy& operator*()
      const& noexcept
    {
      assert_true("Expect contained an error!", is_expect());
      return expected;
    }

    /// @brief Returns the stored Expect value.
    /// @return The Expect value.
    /// @pre is_expected() (colt_expected_is_expected).
    constexpr ExpectedTy& operator*()
      & noexcept
    {
      assert_true("Expect contained an error!", is_expect());
      return expected;
    }
    
    /// @brief Returns the stored Expect value.
    /// @return The Expect value.
    constexpr const ExpectedTy&& operator*()
      const&& noexcept
    {
      assert_true("Expect contained an error!", is_expect());
      return expected;
    }
    
    /// @brief Returns the stored Expect value.
    /// @return The Expect value.
    constexpr ExpectedTy&& operator*()
      && noexcept
    {
      assert_true("Expect contained an error!", is_expect());
      return std::move(expected);
    }

    /// @brief Returns the stored Expect value.
    /// @return The Expect value.
    constexpr const ExpectedTy& value()
      const& noexcept
    {
      assert_true("Expect contained an error!", is_expect());
      return expected;
    }

    /// @brief Returns the stored Expect value.
    /// @return The Expect value.
    constexpr ExpectedTy& value()
      & noexcept
    {
      assert_true("Expect contained an error!", is_expect());
      return expected;
    }
    
    /// @brief Returns the stored Expect value.
    /// @return The Expect value.
    constexpr const ExpectedTy&& value()
      const&& noexcept
    {
      assert_true("Expect contained an error!", is_expect());
      return expected;
    }
    
    /// @brief Returns the stored Expect value.
    /// @return The Expect value.
    constexpr ExpectedTy&& value()
      && noexcept
    {
      assert_true("Expect contained an error!", is_expect());
      return std::move(expected);
    }

    /// @brief Returns the stored error value.
    /// @return The error value.
    constexpr const ErrorTy& error()
      const& noexcept
    {
      assert_true("Expect did not contain an error!", is_error());
      return error_v;
    }

    /// @brief Returns the stored error value.
    /// @return The error value.
    constexpr ErrorTy& error()
      & noexcept
    {
      assert_true("Expect did not contain an error!", is_error());
      return error_v;
    }
    
    /// @brief Returns the stored error value.
    /// @return The error value.
    constexpr const ErrorTy&& error()
      const&& noexcept
    {
      assert_true("Expect did not contain an error!", is_error());
      return error_v;
    }
    
    /// @brief Returns the stored error value.
    /// @return The error value.
    constexpr ErrorTy&& error()
      && noexcept
    {
      assert_true("Expect did not contain an error!", is_error());
      return std::move(error_v);
    }

    /// @brief Returns the Expect value if contained, else 'default_value'
    /// @param default_value The value to return if the Expect contains an error
    /// @return The Expect value or 'default_value'
    constexpr ExpectedTy value_or(ExpectedTy && default_value) const&
    {
      return is_error_v ? static_cast<ExpectedTy>(std::forward<ExpectedTy>(default_value)) : **this;
    }
    /// @brief Returns the Expect value if contained, else 'default_value'
    /// @param default_value The value to return if the Expect contains an error
    /// @return The Expect value or 'default_value'
    constexpr ExpectedTy value_or(ExpectedTy&& default_value)&&
    {
      return is_error_v ? static_cast<ExpectedTy>(std::forward<ExpectedTy>(default_value)) : std::move(**this);
    }

    /// @brief Returns the expected value, or aborts if it does not exist.
    /// @param on_abort The function to call before aborting or null
    /// @return The expected value
    constexpr const ExpectedTy& value_or_abort(void(*on_abort)(void) noexcept = nullptr) const& noexcept
    {
      if (is_error_v)
      {
        if (on_abort)
          on_abort();
        std::abort();
      }
      else
        return expected;
    }

    /// @brief Returns the expected value, or aborts if it does not exist.
    /// @param on_abort The function to call before aborting or null
    /// @return The expected value
    constexpr ExpectedTy& value_or_abort(void(*on_abort)(void) noexcept = nullptr) & noexcept
    {
      if (is_error_v)
      {
        if (on_abort)
          on_abort();
        std::abort();
      }
      else
        return expected;
    }
    
    /// @brief Returns the expected value, or aborts if it does not exist.
    /// @param on_abort The function to call before aborting or null
    /// @return The expected value
    constexpr const ExpectedTy&& value_or_abort(void(*on_abort)(void) noexcept = nullptr) const&& noexcept
    {
      if (is_error_v)
      {
        if (on_abort)
          on_abort();
        std::abort();
      }
      else
        return expected;
    }
    /// @brief Returns the expected value, or aborts if it does not exist.
    /// @param on_abort The function to call before aborting or null
    /// @return The expected value
    constexpr ExpectedTy&& value_or_abort(void(*on_abort)(void) noexcept = nullptr) && noexcept
    {
      if (is_error_v)
      {
        if (on_abort)
          on_abort();
        std::abort();
      }
      else
        return std::move(expected);
    }
  };
}

template<typename T, typename E>
  requires fmt::is_formattable<T>::value && fmt::is_formattable<E>::value
struct fmt::formatter<clt::Expect<T, E>>
  : public clt::meta::DefaultParserFMT
{
  template<typename FormatContext>
  auto format(const clt::Expect<T, E>& exp, FormatContext& ctx)
  {
    auto fmt_to = ctx.out();
    if (exp.is_error())
      return fmt::format_to(fmt_to, "{}", exp.error());
    return fmt::format_to(fmt_to, "{}", exp.value());
  }
};

#endif //!HG_COLT_EXPECT