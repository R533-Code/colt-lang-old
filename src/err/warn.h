/*****************************************************************//**
 * @file   warn.h
 * @brief  Contains WarnFor, that dictates what the AST must warn for.
 * 
 * @author RPC
 * @date   April 2024
 *********************************************************************/
#ifndef HG_COLT_WARN
#define HG_COLT_WARN

#include "util/types.h"

namespace clt::lng
{
	/// @brief What the AST needs to warn for
	struct WarnFor
	{
		/// @brief Warn for variable shadowing
		u8 var_shadowing: 1 = true;
		/// @brief Warn for redundant visibility (public followed public...)
		u8 redundant_visibility: 1 = true;

		/// @brief Return a WarnFor object which warns for everything
		/// @return WarnFor that warns for everything
		static constexpr WarnFor WarnAll() noexcept
		{
			return WarnFor{};
		}
	};
}

#endif // !HG_COLT_WARN