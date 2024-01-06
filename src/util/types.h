#ifndef HG_COLT_TYPES
#define HG_COLT_TYPES

#include <cstdint>
#include <type_traits>
#include "config_type.h"
#include "assertions.h"

namespace clt
{
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
      constexpr T& get() const noexcept { return obj; }
    };
  }

  template<typename T> requires (!std::is_reference_v<T>) && (!std::is_const_v<T>)
  /// @brief An 'out' parameter, which is a uninitialized reference that must be
  /// initialized inside the function. 'out' parameters are taken as is (without const or references).
  using out = std::add_const_t<std::conditional_t<isDebugBuild(), details::OutDebug<T>, details::OutRelease<T>>>&;
}

#endif // !HG_COLT_TYPES