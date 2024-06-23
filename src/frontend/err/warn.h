/*****************************************************************/ /**
 * @file   warn.h
 * @brief  Contains WarnFor, that dictates what the AST must warn for.
 *
 * @author RPC
 * @date   April 2024
 *********************************************************************/
#ifndef HG_COLT_WARN
#define HG_COLT_WARN

#include "common/types.h"

namespace clt::lng
{
  /// @brief What the AST needs to warn for
  struct WarnFor
  {
    /// @brief Warn for variable shadowing
    u8 ast_var_shadowing : 1 = true;
    /// @brief Warn for redundant visibility (public followed public...)
    u8 ast_redundant_visibility : 1 = true;
    /// @brief Warn for NaN as input/output in constant folding
    u8 constant_folding_nan : 1 = true;
    /// @brief Warn for overflow/underflow of signed integer in constant folding
    u8 constant_folding_signed_ou : 1 = true;
    /// @brief Warn for overflow/underflow of unsigned integer in constant folding
    u8 constant_folding_unsigned_ou : 1 = true;
    /// @brief Warn for invalid right/left shift operand
    u8 constant_folding_invalid_shift : 1 = true;

    /// @brief Return a WarnFor object which warns for everything
    /// @return WarnFor that warns for everything
    static constexpr WarnFor warn_all() noexcept { return WarnFor{}; }
  };
} // namespace clt::lng

#endif // !HG_COLT_WARN