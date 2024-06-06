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
    template<typename To, typename From>
    /// @brief Helper to converts a pointer to a type to a pointer to another type
    /// @tparam To The type to convert
    /// @tparam From The type to convert from
    /// @param frm The value to convert
    /// @return Converted value
    constexpr To ptr_to(From frm) noexcept
      requires std::is_pointer_v<To>&& std::is_pointer_v<From>
    {
      return static_cast<To>(
        static_cast<
        meta::match_cv_t<std::remove_pointer_t<From>, void>*
        >(frm));
    }

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

    template<typename T> requires (!std::is_reference_v<T>) && (!std::is_const_v<T>)
      && std::is_trivially_destructible_v<T>
      /// @brief Represents an uninitialized variable (with runtime checks)
      class UninitDebug
    {
      /// @brief The object
      alignas(T) char buffer[sizeof(T)];
      /// @brief Source location of the object
      std::source_location loc;
      /// @brief True if the object was constructed
      mutable bool is_constructed = false;

      constexpr T& val() noexcept
      {
        if (is_constructed)
          return *ptr_to<T*>(buffer);
        clt::unreachable("Use of uninitialized 'uninit' parameter!", loc);
      }

      constexpr const T& val() const noexcept
      {
        if (is_constructed)
          return *ptr_to<const T*>(buffer);
        clt::unreachable("Use of uninitialized 'uninit' parameter!", loc);
      }

    public:
      /// @brief Constructs the out parameter
      /// @param ref The uninitialized object
      /// @param src The source location
      constexpr UninitDebug(std::source_location src = std::source_location::current()) noexcept
        : loc(src) {}

      template<typename... Args>
      constexpr T& init(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
      {
        if (!is_constructed)
        {
          std::construct_at<T>((T*)buffer, std::forward<Args>(args)...);
          is_constructed = true;
          return val();
        }

        clt::unreachable("Double initialization of 'uninit' parameter!", loc);
      }

      /// @brief Initializes the uninitialized memory
      /// @param construct The value to initialize with
      /// @return Reference to the constructed object
      constexpr T& operator=(const T& construct) noexcept(std::is_nothrow_copy_constructible_v<T>)
      {
        return init(construct);
      }

      /// @brief Initializes the uninitialized memory
      /// @param construct The value to initialize with
      /// @return Reference to the constructed object
      constexpr T& operator=(T&& construct) noexcept(std::is_nothrow_copy_constructible_v<T>)
      {
        return init(std::move(construct));
      }

      /// @brief Returns the object (validating that it is constructed)
      /// @return Reference to the object
      [[nodiscard]]
      constexpr const T& get() const noexcept { return val(); }
      
      /// @brief Returns the object (validating that it is constructed)
      /// @return Reference to the object
      [[nodiscard]]
      constexpr T& get() noexcept { return val(); }

      constexpr operator T& () { return val(); }
      constexpr operator const T& () const { return val(); }

      ~UninitDebug()
      {
        if (!is_constructed)
          clt::unreachable("Missing 'init' call of 'uninit' parameter!", loc);
      }
    };

    template<typename T> requires (!std::is_reference_v<T>) && (!std::is_const_v<T>)
      && std::is_trivially_destructible_v<T>
      /// @brief Represents an out parameter (but without any runtime checks)
      class UninitRelease
    {
      /// @brief The object
      alignas(T) char buffer[sizeof(T)];

    public:
      /// @brief Constructs an out parameter
      /// @param ref The object
      constexpr UninitRelease() noexcept {}

      template<typename... Args>
      constexpr T& init(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
      {        
        return *std::construct_at<T>((T*)buffer, std::forward<Args>(args)...);;
      }

      /// @brief Returns the object (validating that it is constructed)
      /// @return Reference to the object
      [[nodiscard]]
      constexpr const T& get() const noexcept { return *ptr_to<const T*>(buffer); }
      /// @brief Returns the object (validating that it is constructed)
      /// @return Reference to the object
      [[nodiscard]]
      constexpr T& get() noexcept { return *ptr_to<T*>(buffer); }

      /// @brief Initializes the uninitialized memory
      /// @param construct The value to initialize with
      /// @return Reference to the constructed object
      constexpr T& operator=(const T& construct) noexcept(std::is_nothrow_copy_constructible_v<T>)
      {
        return *(new(buffer) T(construct));
      }
      
      /// @brief Initializes the uninitialized memory
      /// @param construct The value to initialize with
      /// @return Reference to the constructed object
      constexpr T& operator=(T&& construct) noexcept(std::is_nothrow_copy_constructible_v<T>)
      {
        return *(new(&buffer) T(std::move(construct)));
      }

      constexpr operator T& () { return get(); }
      constexpr operator const T& () const { return get(); }
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

      /// @brief Discards the value
      constexpr void discard() const noexcept { is_checked = true; }

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

      /// @brief Does nothing on Release config
      constexpr void discard() const noexcept {}

      /// @brief Check if the state represents a success
      [[nodiscard]]
      explicit constexpr operator bool() const noexcept { return is_success(); }
    };    
  }

  template<typename T> requires (!std::is_reference_v<T>) && (!std::is_const_v<T>)
  /// @brief An 'out' parameter, which is a uninitialized reference that must be
  /// initialized inside the function. 'out' parameters are taken as is (without const or references).
  /// @tparam T The type of the out parameter
  using out = std::add_const_t<std::conditional_t<isDebugBuild(), details::OutDebug<T>, details::OutRelease<T>>>&;

  template<typename T> requires (!std::is_reference_v<T>) && (!std::is_const_v<T>)
  /// @brief An uninitialized variable.
  /// @tparam T The type of the uninitialized variable
  using uninit = std::conditional_t<isDebugBuild(), details::UninitDebug<T>, details::UninitRelease<T>>;

  /// @brief Boolean that represents a success/failure state that must be checked.
  using ErrorFlag = std::conditional_t<isDebugBuild(), details::ErrorDebug, details::ErrorRelease>;


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
        if constexpr (isDebugBuild())
        {
          assert_true("Only accepted format is {}!",
            ctx.begin() == ctx.end() || *ctx.begin() == '}');
        }        
        return ctx.begin();
      }
    };
  }

  namespace details
  {
    template<typename T> requires std::unsigned_integral<T> && (!std::same_as<T, bool>)
      /// @brief Class representing common byte sizes, simplifying bitwise operations
      class BitSet
    {
      /// @brief Underlying integer storing the value
      T value = 0;

    public:
      /// @brief Constructs a BitSet with all the bits cleared (0)
      constexpr BitSet() noexcept
        : value(0) {}
      /// @brief Constructs a BitSet from an Integer
      /// @param value The value of the bit-set
      explicit constexpr BitSet(T value) noexcept
        : value(value) {}
      constexpr BitSet(const BitSet&)             noexcept = default;
      constexpr BitSet(BitSet&&)                  noexcept = default;
      constexpr BitSet& operator=(const BitSet&)  noexcept = default;
      constexpr BitSet& operator=(BitSet&&)       noexcept = default;

      /// @brief Returns the state of the nth-bit
      /// @param n The index of the bit (starting at 0)
      /// @return True if on (1)
      constexpr bool operator[](size_t n)  const noexcept { return (value >> n) & 1U; }
      /// @brief Check if the nth-bit is set (1)
      /// @param n The index of the bit (starting at 0)
      /// @return True if on (1)
      constexpr bool is_set(size_t n)     const noexcept { return (value >> n) & 1U; }
      /// @brief Check if the nth-bit is clear (0)
      /// @param n The index of the bit (starting at 0)
      /// @return True if off (0)
      constexpr bool is_clr(size_t n)     const noexcept { return !is_set(n); }
      /// @brief Check if all the bits are set
      /// @return True if all the bits are on (1)
      constexpr bool is_all_set()         const noexcept { return value == (T)(-1); }
      /// @brief Check if at least a bit is set
      /// @return True if at least a bit is on (1)
      constexpr bool is_any_set()         const noexcept { return std::popcount(value) != 0; }
      /// @brief Check if no bit is set
      /// @return True if no bit is on (1)
      constexpr bool is_none_set()        const noexcept { return value == 0; }

      /// @brief Sets the state of the nth-bit to 1
      /// @param n The index of the bit (starting at 0)
      constexpr void set_n(size_t n) noexcept
      {
        assert_true("Invalid index!", n < sizeof(T) * 8);
        value |= 1UL << n;
      }

      /// @brief Clears the state of the nth-bit (sets the bit to 0)
      /// @param n The index of the bit (starting at 0)
      constexpr void clr_n(size_t n) noexcept
      {
        assert_true("Invalid index!", n < sizeof(T) * 8);
        value &= ~(1UL << n);
      }

      /// @brief Toggles the state of the nth-bit (if 0 -> 1, if 1 -> 0)
      /// @param n The index of the bit (starting at 0)
      constexpr void tgl_n(size_t n) noexcept
      {
        assert_true("Invalid index!", n < sizeof(T) * 8);
        value ^= 1UL << n;
      }

      /// @brief Changes the state of the nth-bit to 'to'
      /// @param n The index of the bit (starting at 0)
      /// @param to The value to set the bit to
      constexpr void chg_n(size_t n, bool to) noexcept
      {
        assert_true("Invalid index!", n < sizeof(T) * 8);
        value ^= (-(T)to ^ value) & (1UL << n);
      }

      /// @brief Bitwise OR operator
      /// @param byte The bit-set with which to perform the operation
      /// @return New bit-set containing the result of the operation
      constexpr BitSet operator|(BitSet byte) const noexcept { return value | byte.value; }
      /// @brief Bitwise XOR operator
      /// @param byte The bit-set with which to perform the operation
      /// @return New bit-set containing the result of the operation
      constexpr BitSet operator^(BitSet byte) const noexcept { return value ^ byte.value; }
      /// @brief Bitwise AND operator
      /// @param byte The bit-set with which to perform the operation
      /// @return New bit-set containing the result of the operation
      constexpr BitSet operator&(BitSet byte) const noexcept { return value & byte.value; }
      /// @brief Shift Left operator
      /// @param by By how many bits to shift
      /// @return New bit-set containing the result of the operation
      constexpr BitSet operator<<(size_t by) const noexcept
      {
        assert_true("Invalid bit-shift!", by < sizeof(T) * 8);
        return value << by;
      }

      /// @brief Shift Right operator
      /// @param by By how many bits to shift
      /// @return New bit-set containing the result of the operation
      constexpr BitSet operator>>(size_t by) const noexcept
      {
        assert_true("Invalid bit-shift!", by < sizeof(T) * 8);
        return value >> by;
      }

      /// @brief Bitwise NOT operator
      /// @return New bit-set containing the result of the operation
      constexpr BitSet operator~() const noexcept { return ~value; }

      /// @brief Bitwise OR operator
      /// @param byte The bit-set with which to perform the operation
      /// @return Self
      constexpr BitSet& operator|=(BitSet byte) noexcept { value |= byte.value; return *this; }
      /// @brief Bitwise XOR operator
      /// @param byte The bit-set with which to perform the operation
      /// @return Self
      constexpr BitSet& operator^=(BitSet byte) noexcept { value ^= byte.value; return *this; }
      /// @brief Bitwise AND operator
      /// @param byte The bit-set with which to perform the operation
      /// @return Self
      constexpr BitSet& operator&=(BitSet byte) noexcept { value &= byte.value; return *this; }
      /// @brief Shift Left operator
      /// @param by By how many bits to shift
      /// @return Self
      constexpr BitSet& operator<<=(size_t by) noexcept
      {
        assert_true("Invalid bit-shift!", by < sizeof(T) * 8);
        value <<= by; return *this;
      }

      /// @brief Shift Right operator
      /// @param by By how many bits to shift
      /// @return Self
      constexpr BitSet& operator>>=(size_t by) noexcept
      {
        assert_true("Invalid bit-shift!", by < sizeof(T) * 8);
        value >>= by; return *this;
      }

      /// @brief Counts the number of set bits
      /// @return The number of bits that are set (=1)
      constexpr size_t count() const noexcept { return std::popcount(value); }

      /// @brief Returns the underlying integer storing the bit-set
      /// @return Reference to the underlying integer
      constexpr T& to_underlying() noexcept { return value; }
      /// @brief Returns the underlying integer storing the bit-set
      /// @return Reference to the underlying integer
      constexpr const T& to_underlying() const noexcept { return value; }

      /// @brief Sets all the bits to 0
      /// @return Self
      constexpr BitSet& clear() noexcept { value = 0; return *this; }

      template<typename From> requires (sizeof(From) <= sizeof(T))
      /// @brief Assigns the bitwise representation of 'frm'
      /// @param frm The value whose bitwise representation to use
      /// @return Self
      constexpr BitSet bit_assign(From frm) noexcept
      {
        std::memcpy(&value, &frm, sizeof(From));
        return *this;
      }

      template<typename To> requires (sizeof(To) <= sizeof(T))
      /// @brief Converts the underlying bits to an object of 'To'
      /// @tparam To The type to convert to
      /// @return The converted object
      constexpr To as() const noexcept
      {
        To ret;
        std::memcpy(&ret, &value, sizeof(To));
        return ret;
      }

      /// @brief Underlying integer type storing the value
      using underlying_type = T;
    };
  }

  /// @brief 8-bit BitSet
  using BYTE_t = details::BitSet<u8>;
  /// @brief 16-bit BitSet
  using WORD_t = details::BitSet<u16>;
  /// @brief 32-bit BitSet
  using DWORD_t = details::BitSet<u32>;
  /// @brief 64-bit BitSet
  using QWORD_t = details::BitSet<u64>;

  template<typename T>
  /// @brief Check if a type is one of BYTE_t/WORD_t/DWORD_t/QWORD_t
  concept BitType = std::same_as<T, BYTE_t>
    || std::same_as<T, WORD_t>
    || std::same_as<T, DWORD_t>
    || std::same_as<T, QWORD_t>;

  template<BitType T, typename Wt> requires (sizeof(Wt) == sizeof(T))
  /// @brief Converts a type to its bit-set equivalent
  /// @tparam Wt The type to convert
  /// @tparam T One of [BYTE, WORD, DWORD, QWORD]
  /// @param what The value to convert
  /// @return The converted value
  constexpr T bit_as(const Wt& what) noexcept
  {
    using cnv_t = typename T::underlying_type;
    return T(std::bit_cast<cnv_t>(what));
  }
}

namespace clt::details
{
  template<typename Fun>
  /// @brief Helper for ON_SCOPE_EXIT
  struct ScopeGuard
  {
    /// @brief The function to execute
    Fun fn;

    /// @brief Takes in a lambda to execute
    /// @param fn The function to execute
    ScopeGuard(Fun&& fn) noexcept
      : fn(std::forward<Fun>(fn)) {}

    /// @brief Runs the lambda in the destructor
    ~ScopeGuard() noexcept { fn(); }
  };

  /// @brief Helper for ON_SCOPE_EXIT
  enum class ScopeGuardOnExit {};

  template<typename Fun>
  /// @brief Helper for ON_SCOPE_EXIT
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