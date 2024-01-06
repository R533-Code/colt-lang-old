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

namespace clt::meta
{
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
}

#endif // !HG_META_TRAITS