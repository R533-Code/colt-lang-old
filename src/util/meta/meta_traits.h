/*****************************************************************//**
 * @file   meta_traits.h
 * @brief  Contains some meta-programming helpers.
 * 
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_META_TRAITS
#define HG_META_TRAITS

#include <type_traits>
#include "common/config_type.h"

namespace clt::meta
{
  /// @brief Empty struct helper
  struct Empty {};

  template<typename Of, typename For>
  /// @brief Makes a type match the const-ness of another
  /// @tparam Of The type whose const-ness to match against
  /// @tparam For The type to which to add const if necessary
  struct match_const
  {
    using type = std::conditional_t<std::is_const_v<Of>, std::add_const_t<For>, std::remove_const_t<For>>;
  };

  template<typename Of, typename For>
  /// @brief Short-hand for match_const<Of, For>::type
  /// @tparam Of The type whose const-ness to match against
  /// @tparam For The type to which to add const if necessary
  using match_const_t = typename match_const<Of, For>::type;

  template<typename Of, typename For>
  /// @brief Makes a type match the volatile-ness of another
  /// @tparam Of The type whose volatile-ness to match against
  /// @tparam For The type to which to add volatile if necessary
  struct match_volatile
  {
    using type = std::conditional_t<std::is_volatile_v<Of>, std::add_volatile_t<For>, std::remove_volatile_t<For>>;
  };

  template<typename Of, typename For>
  /// @brief Short-hand for match_volatile<Of, For>::type
  /// @tparam Of The type whose volatile-ness to match against
  /// @tparam For The type to which to add volatile if necessary
  using match_volatile_t = typename match_volatile<Of, For>::type;

  template<typename Of, typename For>
  /// @brief Makes a type match the qualifiers of another
  /// @tparam Of The type whose qualifiers to match against
  /// @tparam For The type to which to add const if necessary
  struct match_cv
  {
    using type = match_volatile_t<Of, match_const_t<Of, For>>;
  };

  template<typename Of, typename For>
  /// @brief Short-hand for match_cv<Of, For>::type
  /// @tparam Of The type whose qualifiers to match against
  /// @tparam For The type to transform if necessary
  using match_cv_t = typename match_cv<Of, For>::type;

  namespace details
  {
    template<typename T> requires (std::is_void_v<T>)
      /// @brief Helper for sizeof_or_zero
      /// @tparam T The type to check for
      /// @return 0
      consteval size_t sizeof_or_zero() noexcept
    {
      return 0;
    }

    template<typename T> requires (!std::is_void_v<T>)
      /// @brief Helper for sizeof_or_zero
      /// @tparam T The type to check for
      /// @return sizeof(T)
      consteval size_t sizeof_or_zero() noexcept
    {
      return sizeof(T);
    }
  }

  template<typename T>
  /// @brief Returns the sizeof a type or 0 if void
  /// @tparam T The type
  struct sizeof_or_zero
  {
    static constexpr size_t value = details::sizeof_or_zero<T>();
  };

  template<typename T>
  /// @brief Shorthand for sizeof_or_zero<T>::value
  /// @tparam T The type to check for
  inline constexpr size_t sizeof_or_zero_v = sizeof_or_zero<T>::value;

  template<typename Debug, typename Release>
  /// @brief Chooses a type if on Debug configuration, or another for Release
  /// @tparam Debug The type on Debug configuration
  /// @tparam Release The type on Release configuration
  struct for_debug_for_release
  {
    using type = std::conditional_t<clt::isDebugBuild(), Debug, Release>;
  };

  template<typename Debug, typename Release>
  /// @brief Shorthand for 'for_debug_for_release<Debug, Release>::type'
  /// @tparam Debug The type on Debug configuration
  /// @tparam Release The type on Release configuration
  using for_debug_for_release_t = typename for_debug_for_release<Debug, Release>::type;
}

#endif // !HG_META_TRAITS