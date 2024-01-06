/*****************************************************************//**
 * @file   config_type.h
 * @brief  Contains `isDebugBuild` function.
 * 
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_CONFIG_TYPE
#define HG_COLT_CONFIG_TYPE

#include "colt_config.h"

namespace clt
{
#ifdef COLT_DEBUG
  /// @brief Check if the current build is on DEBUG configuration
  /// @return True if on DEBUG
  consteval bool isDebugBuild() noexcept { return true; }
#else
  /// @brief Check if the current build is on DEBUG configuration
  /// @return True if on DEBUG
  consteval bool isDebugBuild() noexcept { return false; }
#endif // COLT_DEBUG

  /// @brief Check if the current build is on RELEASE configuration
  /// @return True if on RELEASE (not on DEBUG)
  consteval bool isReleaseBuild() noexcept { return !isDebugBuild(); }
}

#endif // !HG_COLT_CONFIG_TYPE
