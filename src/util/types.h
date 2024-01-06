/*****************************************************************//**
 * @file   types.h
 * @brief  Contains useful typedefs.
 * The `out` type can be used to mark parameters as being uninitialized.
 * These parameter must then be initialized before the function returns.
 * @code{.cpp}
 * void foo(out<int> a)
 * {
 *   //...
 *   a.init(10);
 * }
 * @endcode
 * 
 * The `Error` type is a simple boolean flag that represents either a
 * success or a failure state. An `Error` must be checked before its
 * destructor runs.
 * 
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_TYPES
#define HG_COLT_TYPES

#include <cstdint>
#include <type_traits>
#include "meta/meta_traits.h"
#include "config_type.h"
#include "assertions.h"

 /// @brief 8-bit signed integer
using u8 = uint8_t;
/// @brief 16-bit signed integer
using u16 = uint16_t;
/// @brief 32-bit signed integer
using u32 = uint32_t;
/// @brief 64-bit unsigned integer
using u64 = uint64_t;

/// @brief 8-bit signed integer
using i8 = int8_t;
/// @brief 16-bit signed integer
using i16 = int16_t;
/// @brief 32-bit signed integer
using i32 = int32_t;
/// @brief 64-bit signed integer
using i64 = int64_t;

/// @brief single precision floating point
using f32 = float;
/// @brief double precision floating point
using f64 = double;

namespace clt
{
  namespace details
  {
    template<typename T> requires (!std::is_reference_v<T>) && (!std::is_const_v<T>)
    /// @brief Represents an out parameter (with runtime checks)
    class OutDebug
    {
      /// @brief The object
      T& obj;
      /// @brief Source location of the object
      std::source_location loc;
      /// @brief True if the object was constructed
      mutable bool is_constructed = false;

    public:
      /// @brief Constructs the out parameter
      /// @param ref The uninitialized object
      /// @param src The source location
      constexpr OutDebug(T& ref, std::source_location src = std::source_location::current()) noexcept
        : obj(ref), loc(src) {}

      template<typename... Args>
      constexpr T& init(Args&&... args) const noexcept(std::is_nothrow_constructible_v<T, Args...>)
      {
        if (!is_constructed)
        {
          new(&obj) T(std::forward<Args>(args)...);
          is_constructed = true;
          return obj;
        }
        
        clt::unreachable("Double initialization of 'out' parameter!", loc);
      }

      /// @brief Returns the object (validating that it is constructed)
      /// @return Reference to the object
      [[nodiscard]]
      constexpr T& get() const noexcept
      {
        if (is_constructed)
          return obj;
        
        clt::unreachable("Use of uninitialized 'out' parameter!", loc);
      }

      ~OutDebug()
      {
        if (!is_constructed)
          clt::unreachable("Missing 'init' call of 'out' parameter!", loc);
      }
    };

    template<typename T> requires (!std::is_reference_v<T>) && (!std::is_const_v<T>)
    /// @brief Represents an out parameter (but without any runtime checks)
    class OutRelease
    {
      /// @brief The object
      T& obj;

    public:
      /// @brief Constructs an out parameter
      /// @param ref The object
      constexpr OutRelease(T& ref) noexcept
        : obj(ref) {}

      template<typename... Args>
      constexpr T& init(Args&&... args) const noexcept(std::is_nothrow_constructible_v<T, Args...>)
      {
        new(&obj) T(std::forward<Args>(args)...);
        return obj;
      }

      /// @brief Returns the object (validating that it is constructed)
      /// @return Reference to the object
      [[nodiscard]]
      constexpr T& get() const noexcept { return obj; }
    };

    /// @brief Represents an Error (with runtime checks)
    class ErrorDebug
    {
      /// @brief The source location
      std::source_location src;
      /// @brief True if represents an error
      bool iserror;
      /// @brief True if the error state was read at least once
      mutable bool is_checked = false;

      /// @brief Constructs an error with a specific state.
      /// The constructor is marked private so that the more readable sucess and error
      /// methods are used.
      /// @param is_error True if an error
      ErrorDebug(bool is_error, const std::source_location& src)
        : src(src), iserror(is_error) {}

      /// @brief Asserts that the state was read at least once
      void assert_checked() noexcept
      {
        if (!is_checked)
          clt::unreachable("'Error' result must be checked!", src);
      }

    public:
      ErrorDebug(const ErrorDebug&) = delete;
      ErrorDebug operator=(const ErrorDebug&) = delete;

      // Move Constructor (steals the state of 'move')
      ErrorDebug(ErrorDebug&& move) noexcept
        : iserror(move.iserror), is_checked(std::exchange(move.is_checked, true)) {}
      // Move assignment operator
      ErrorDebug& operator=(ErrorDebug&& move) noexcept
      {
        assert_checked();
        iserror = move.iserror;
        is_checked = std::exchange(move.is_checked, true);
      }

      /// @brief Constructs a success
      /// @return State representing a success
      [[nodiscard]]
      static auto success(std::source_location src = std::source_location::current()) noexcept { return ErrorDebug(false, src); }
      /// @brief Constructs an error
      /// @return State representing an error
      [[nodiscard]]
      static auto error(std::source_location src = std::source_location::current()) noexcept { return ErrorDebug(true, src); }

      /// @brief Check if the state represents an error
      [[nodiscard]]
      constexpr bool is_error() const noexcept
      {
        is_checked = true;
        return iserror;
      }      
      /// @brief Check if the state represents a success
      [[nodiscard]]
      constexpr bool is_success() const noexcept { return !is_error(); }

      /// @brief Check if the state represents a success
      [[nodiscard]]
      explicit constexpr operator bool() const noexcept { return is_success(); }

      /// @brief Ensures the state was at least read once
      ~ErrorDebug()
      {
        assert_checked();
      }
    };

    /// @brief Represents an Error (without any runtime checks)
    class ErrorRelease
    {
      /// @brief True if error
      bool iserror;

      /// @brief Constructs an error with a specific state.
      /// The constructor is marked private so that the more readable sucess and error
      /// methods are used.
      /// @param is_error True if an error
      constexpr ErrorRelease(bool is_error)
        : iserror(is_error) {}

    public:
      ErrorRelease(const ErrorRelease&) = delete;
      ErrorRelease& operator=(const ErrorRelease&) = delete;
      
      // Default move constructor
      constexpr ErrorRelease(ErrorRelease&&) noexcept = default;
      // Default move assignment operator
      constexpr ErrorRelease& operator=(ErrorRelease&&) noexcept = default;

      /// @brief Constructs a success
      /// @return State representing a success
      [[nodiscard]]
      constexpr static auto success() noexcept { return ErrorRelease(false); }
      /// @brief Constructs an error
      /// @return State representing an error
      [[nodiscard]]
      constexpr static auto error() noexcept { return ErrorRelease(true); }

      /// @brief Check if the state represents an error
      [[nodiscard]]
      constexpr bool is_error() const noexcept { return iserror; }
      /// @brief Check if the state represents a success
      [[nodiscard]]
      constexpr bool is_success() const noexcept { return !iserror; }

      /// @brief Check if the state represents a success
      [[nodiscard]]
      explicit constexpr operator bool() const noexcept { return is_success(); }
    };

    template<typename To, typename From>
    /// @brief Helper to converts a pointer to a type to a pointer to another type
    /// @tparam To The type to convert
    /// @tparam From The type to convert from
    /// @param frm The value to convert
    /// @return Converted value
    constexpr To ptr_to(From frm) noexcept
      requires std::is_pointer_v<To> && std::is_pointer_v<From>
    {
      return static_cast<To>(
        static_cast<
        meta::match_cv_t<std::remove_pointer_t<From>, void>*
        >(frm));
    }
  }

  template<typename T> requires (!std::is_reference_v<T>) && (!std::is_const_v<T>)
  /// @brief An 'out' parameter, which is a uninitialized reference that must be
  /// initialized inside the function. 'out' parameters are taken as is (without const or references).
  using out = std::add_const_t<std::conditional_t<isDebugBuild(), details::OutDebug<T>, details::OutRelease<T>>>&;

  /// @brief Boolean that represents a success/failure state that must be checked.
  using Error = std::conditional_t<isDebugBuild(), details::ErrorDebug, details::ErrorRelease>;

  namespace meta
  {
    template<typename T>
    /// @brief Unsigned integral
    concept UnsignedIntegral = std::same_as<std::remove_cv_t<T>, u8>
      || std::same_as<std::remove_cv_t<T>, u16>
      || std::same_as<std::remove_cv_t<T>, u32>
      || std::same_as<std::remove_cv_t<T>, u64>;

    template<typename T>
    /// @brief Signed integral
    concept SignedIntegral = std::same_as<std::remove_cv_t<T>, i8>
      || std::same_as<std::remove_cv_t<T>, i16>
      || std::same_as<std::remove_cv_t<T>, i32>
      || std::same_as<std::remove_cv_t<T>, i64>;

    template<typename T>
    /// @brief Signed/Unsigned integral
    concept Integral = UnsignedIntegral<T> || SignedIntegral<T>;

    template<typename T>
    /// @brief Floating point (f32, f64)
    concept FloatingPoint = std::same_as<std::remove_cv_t<T>, f32>
      || std::same_as<std::remove_cv_t<T>, f64>;

    /// @brief Let fmt::formatter specialization inherit from this type if
    /// they only accept an empty format specification.
    struct DefaultParserFMT
    {
      template<typename ParseContext>
      constexpr auto parse(ParseContext& ctx)
      {
        assert_true("Only accepted format is {}!", ctx.begin() == ctx.end());
        return ctx.begin();
      }
    };
  }
}

namespace clt::details
{
  template<typename Fun>
  struct ScopeGuard
  {
    Fun fn;

    ScopeGuard(Fun&& fn) noexcept
      : fn(std::forward<Fun>(fn)) {}

    ~ScopeGuard() noexcept { fn(); }
  };

  enum class ScopeGuardOnExit {};

  template<typename Fun>
  ScopeGuard<Fun> operator+(ScopeGuardOnExit, Fun&& fn) noexcept
  {
    return ScopeGuard<Fun>(std::forward<Fun>(fn));
  }
}

/// @brief Register an action to be called at the end of the scope.
/// Example:
/// @code{.cpp}
/// {
///		auto hello = 10;
///		ON_SCOPE_EXIT {
///			std::cout << hello;
///		}; // <- do not forget the semicolon
/// }
/// @endcode
#define ON_SCOPE_EXIT auto COLT_CONCAT(SCOPE_EXIT_HELPER, __LINE__) = clt::details::ScopeGuardOnExit() + [&]() 

#endif // !HG_COLT_TYPES