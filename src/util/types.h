#ifndef HG_COLT_TYPES
#define HG_COLT_TYPES

#include <cstdint>
#include <type_traits>
#include "config_type.h"

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
    class OutDebug
    {
      T& obj;
      bool is_constructed = false;

    public:
      constexpr OutDebug(T& ref) noexcept
        : obj(ref) {}

      template<typename... Args>
      constexpr T& init(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
      {
        new(&obj) T(std::forward<Args>(args)...);
        is_constructed = true;
      }

      ~OutDebug()
      {
        
      }
    };

    template<typename T> requires (!std::is_reference_v<T>) && (!std::is_const_v<T>)
    class OutRelease
    {

    };
  }
}

#endif // !HG_COLT_TYPES
