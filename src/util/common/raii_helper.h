/*****************************************************************//**
 * @file   raii_helper.h
 * @brief  Contains RAIIResource, helper to manage RAII resources.
 * RAIIResource is simply a unique_ptr with a custom deleter.
 * 
 * @author RPC
 * @date   June 2024
 *********************************************************************/
#ifndef HG_CLT_RAII_HELPER
#define HG_CLT_RAII_HELPER

#include <memory>

namespace clt
{
  template <auto fn>
  /// @brief Custom deleter for unique pointer
  struct deleter_from_fn
  {
    template <typename T>
    constexpr void operator()(T* arg) const { fn(arg); }
  };

  template <typename T, auto fn>
  /// @brief Resource that is automatically closed
  using RAIIResource = std::unique_ptr<T, deleter_from_fn<fn>>;
}

#endif // !HG_CLT_RAII_HELPER
