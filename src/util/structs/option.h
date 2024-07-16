/*****************************************************************/ /**
 * @file   option.h
 * @brief  Contains `Option`, which is either a value or None.
 *
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_OPTIONAL
#define HG_COLT_OPTIONAL

#include "fmt/format.h"
#include "common/types.h"
#include "lex/ascii.h"

namespace clt
{
  namespace meta
  {
    template<typename T>
    /// @brief Check if a type is parsable from a string
    concept Parsable = requires(T a) { scn::scan_default("10", a); };

    /// @brief Tag struct for constructing an object in place
    struct InPlaceT
    {
    };
  } // namespace meta

  /// @brief Tag object for construting an object in place
  inline constexpr meta::InPlaceT InPlace;  

  template<typename T>
  /// @brief Manages an optionally contained value.
  /// @tparam T The optional type to hold
  class Option
  {
    /// @brief Buffer for the optional object
    alignas(T) char opt_buffer[sizeof(T)];
    /// @brief True if no object is contained
    bool is_none_v;

  public:
    /// @brief Destroy the stored value if it exists, and sets the Option to an empty one.
    /// Called automatically by the destructor.
    constexpr void reset() noexcept(std::is_nothrow_destructible_v<T>)
    {
      if (!is_none_v)
      {
        ptr_to<T*>(opt_buffer)->~T();
        is_none_v = true;
      }
    }

    /// @brief Constructs an empty Option.
    constexpr Option() noexcept
        : is_none_v(true)
    {
    }

    /// @brief Constructs an empty Option.
    /// Same as Option().
    /// @param  NoneT: use None
    constexpr Option(details::NoneT) noexcept
        : is_none_v(true)
    {
    }

    /// @brief Copy constructs an object into the Option.
    /// @param to_copy The object to copy
    constexpr Option(const T& to_copy) noexcept(
        std::is_nothrow_copy_constructible_v<T>)
        : is_none_v(false)
    {
      new (opt_buffer) T(to_copy);
    }

    /// @brief Move constructs an object into the Option
    /// @param to_move The object to move
    constexpr Option(T&& to_move) noexcept(std::is_nothrow_move_constructible_v<T>)
      requires(!std::is_trivial_v<T>)
        : is_none_v(false)
    {
      new (opt_buffer) T(std::move(to_move));
    }

    template<typename... Args>
    /// @brief Constructs an object into the Option directly.
    /// @tparam ...Args The parameter pack
    /// @param  InPlaceT, use InPlace
    /// @param ...args The argument pack
    constexpr Option(meta::InPlaceT, Args&&... args) noexcept(
        std::is_nothrow_constructible_v<T, Args...>)
        : is_none_v(false)
    {
      new (opt_buffer) T(std::forward<Args>(args)...);
    }

    /// @brief Copy constructor.
    /// @param to_copy The Option to copy
    constexpr Option(const Option& to_copy) noexcept(
        std::is_nothrow_copy_constructible_v<T>)
        : is_none_v(to_copy.is_none_v)
    {
      if (!is_none_v)
        new (opt_buffer) T(*ptr_to<const T*>(to_copy.opt_buffer));
    }

    /// @brief Move constructor.
    /// @param to_move The Option to move
    constexpr Option(Option&& to_move) noexcept(
        std::is_nothrow_move_constructible_v<T>)
        : is_none_v(to_move.is_none_v)
    {
      if (!is_none_v)
        new (opt_buffer) T(std::move(*ptr_to<T*>(to_move.opt_buffer)));
    }

    /// @brief Copy assignment operator
    /// @param to_copy The optional to copy
    /// @return Self
    constexpr Option& operator=(const Option& to_copy) noexcept(
        std::is_nothrow_destructible_v<T> && std::is_nothrow_copy_constructible_v<T>)
    {
      assert_true("Self assignment is prohibited!", &to_copy != this);
      reset();
      if (to_copy.is_value())
      {
        is_none_v = false;
        new (opt_buffer) T(*ptr_to<const T*>(to_copy.opt_buffer));
      }
      return *this;
    }

    /// @brief Move assignment operator
    /// @param to_move The optional to move
    /// @return Self
    constexpr Option& operator=(Option&& to_move) noexcept(
        std::is_nothrow_destructible_v<T> && std::is_nothrow_move_constructible_v<T>)
    {
      assert_true("Self assignment is prohibited!", &to_move != this);
      reset();
      if (to_move.is_value())
      {
        is_none_v = false;
        new (opt_buffer) T(std::move(*ptr_to<T*>(to_move.opt_buffer)));
      }
      return *this;
    }

    /// @brief Copy a value to the optional
    /// @param to_copy The value to copy
    /// @return Self
    constexpr Option& operator=(const T& to_copy) noexcept(
        std::is_nothrow_destructible_v<T> && std::is_nothrow_copy_constructible_v<T>)
    {
      reset();
      is_none_v = false;
      new (opt_buffer) T(to_copy);
      return *this;
    }

    /// @brief Move a value to the optional
    /// @param to_move The value to move
    /// @return Self
    constexpr Option& operator=(T&& to_move) noexcept(
        std::is_nothrow_destructible_v<T> && std::is_nothrow_move_constructible_v<T>)
    {
      reset();
      is_none_v = false;
      new (opt_buffer) T(std::move(to_move));
      return *this;
    }

    /// @brief Resets the Option.
    /// Same as `reset()`.
    /// @param  Error type (Error)
    /// @return Reference to self
    constexpr Option& operator=(details::NoneT) noexcept(
        std::is_nothrow_destructible_v<T>)
    {
      reset();
      return *this;
    }

    /// @brief Destructor, destructs the value if it exist.
    constexpr ~Option() noexcept(std::is_nothrow_destructible_v<T>) { reset(); }

    /// @brief Check if the Option contains a value.
    /// @return True if the Option contains a value
    explicit constexpr operator bool() const noexcept { return !is_none_v; }

    /// @brief Check if the Option contains a value.
    /// Same as !is_none().
    /// @return True if the Option contains a value
    constexpr bool is_value() const noexcept { return !is_none_v; }

    /// @brief Check if the Option does not contain a value.
    /// Same as !is_value().
    /// @return True if the Option does not contain a value
    constexpr bool is_none() const noexcept { return is_none_v; }

    /// @brief Returns the stored value.
    /// @return The value
    constexpr const T* operator->() const noexcept
    {
      assert_true("Option does not contain a value!", is_value());
      return ptr_to<const T*>(opt_buffer);
    }

    /// @brief Returns the stored value.
    /// @return The value
    constexpr T* operator->() noexcept
    {
      assert_true("Option does not contain a value!", is_value());
      return ptr_to<T*>(opt_buffer);
    }

    /// @brief Returns the stored value.
    /// @return The value.
    constexpr const T& operator*() const& noexcept
    {
      assert_true("Option does not contain a value!", is_value());
      return *ptr_to<const T*>(opt_buffer);
    }

    /// @brief Returns the stored value.
    /// @return The value.
    constexpr T& operator*() & noexcept
    {
      assert_true("Option does not contain a value!", is_value());
      return *ptr_to<T*>(opt_buffer);
    }

    /// @brief Returns the stored value.
    /// @return The value.
    constexpr const T&& operator*() const&& noexcept
    {
      assert_true("Option does not contain a value!", is_value());
      return *ptr_to<const T*>(opt_buffer);
    }

    /// @brief Returns the stored value.
    /// @return The value.
    constexpr T&& operator*() && noexcept
    {
      assert_true("Option does not contain a value!", is_value());
      return std::move(*ptr_to<T*>(opt_buffer));
    }

    /// @brief Returns the stored value.
    /// @return The value.
    constexpr const T& value() const& noexcept
    {
      assert_true("Option does not contain a value!", is_value());
      return *ptr_to<const T*>(opt_buffer);
    }

    /// @brief Returns the stored value.
    /// @return The value.
    constexpr T& value() & noexcept
    {
      assert_true("Option does not contain a value!", is_value());
      return *ptr_to<T*>(opt_buffer);
    }

    /// @brief Returns the stored value.
    /// @return The value.
    constexpr const T&& value() const&& noexcept
    {
      assert_true("Option does not contain a value!", is_value());
      return *ptr_to<const T*>(opt_buffer);
    }

    /// @brief Returns the stored value.
    /// @return The value.
    /// @pre is_value().
    constexpr T&& value() && noexcept
    {
      assert_true("Option does not contain a value!", is_value());
      return std::move(*ptr_to<T*>(opt_buffer));
    }

    /// @brief Returns the value if contained, else 'default_value'
    /// @param default_value The value to return if the Option is None
    /// @return The value or 'default_value'
    constexpr T value_or(T&& default_value) const&
    {
      return is_none_v ? static_cast<T>(std::forward<T>(default_value)) : **this;
    }

    /// @brief Returns the value if contained, else 'default_value'
    /// @param default_value The value to return if the Option is None
    /// @return The value or 'default_value'
    constexpr T value_or(T&& default_value) &&
    {
      return is_none_v ? static_cast<T>(std::forward<T>(default_value))
                       : std::move(**this);
    }
  };
} // namespace clt

template<clt::meta::Parsable T>
struct scn::scanner<clt::Option<T>> : scn::empty_parser
{
  template<typename Context>
  error scan(clt::Option<T>& val, Context& ctx)
  {
    std::string_view strv;
    auto r = scn::scan(ctx.range(), "{}", strv);
    ON_SCOPE_EXIT
    {
      ctx.range() = std::move(r.range());
    };
    if (r && clt::is_equal_case_insensitive("none", strv))
    {
      val = clt::None;
      return r.error();
    }
    T value;
    r   = scn::scan(ctx.range(), "{}", value);
    val = value;
    return r.error();
  }
};

template<typename T>
  requires fmt::is_formattable<T>::value
/// @brief Formatter for an optional
struct fmt::formatter<clt::Option<T>>
{
  /// @brief Default string to print if empty optional
  const char* none_str = "None";
  /// @brief Size of the string to print if empty optional
  size_t none_size = 4;

  template<typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    auto it = ctx.begin();
    if (it == ctx.end())
      return it;

    // Parse the string to print if empty optional
    none_str  = it;
    none_size = 0;
    while (*it != '}')
      ++none_size, ++it;

    return it;
  }

  template<typename FormatContext>
  auto format(const clt::Option<T>& opt, FormatContext& ctx)
  {
    auto fmt_to = ctx.out();
    if (opt.is_value())
      return fmt::format_to(fmt_to, "{}", opt.value());
    else
      return fmt::format_to(fmt_to, "{:.{}}", none_str, none_size);
  }
};

#endif //!HG_COLT_OPTIONAL