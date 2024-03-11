/*****************************************************************//**
 * @file   colt_type_buffer.cpp
 * @brief  Contains the implementation of `colt_type_buffer.h`.
 * 
 * @author RPC
 * @date   March 2024
 *********************************************************************/
#include "colt_type_buffer.h"

namespace clt::lng
{
#ifdef COLT_DEBUG
  // Used on debug to verify that a token is for this TypeBuffer
  std::atomic<u32> TypeBuffer::ID_GENERATOR{};
#endif // COLT_DEBUG
}
