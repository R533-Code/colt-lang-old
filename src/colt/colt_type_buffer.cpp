#ifndef HG_COLTC_TYPE_BUFFER
#define HG_COLTC_TYPE_BUFFER

#include "colt_type.h"
#include "structs/map.h"

namespace clt::lng
{
#ifdef COLT_DEBUG
  std::atomic<u32> TypeBuffer::ID_GENERATOR{};
#endif // COLT_DEBUG
}

#endif // !HG_COLTC_TYPE_BUFFER
