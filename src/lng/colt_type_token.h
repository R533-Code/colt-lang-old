/*****************************************************************//**
 * @file   colt_type_token.h
 * @brief  Contains TypeToken, a token that represents a Colt type.
 * 
 * @author RPC
 * @date   April 2024
 *********************************************************************/
#ifndef HG_COLTC_TYPE_TOKEN
#define HG_COLTC_TYPE_TOKEN

#include "util/types.h"
#include "util/token_helper.h"

namespace clt::lng
{
  class TypeBuffer;

  /// @brief Represent a type through its index
  CREATE_TOKEN_TYPE(TypeToken, u32, std::numeric_limits<u32>::max(),
    TypeBuffer);
}

#endif // !HG_COLTC_TYPE_TOKEN
