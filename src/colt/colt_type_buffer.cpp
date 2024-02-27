#include "colt_type_buffer.h"

namespace clt::lng
{
#ifdef COLT_DEBUG
  std::atomic<u32> TypeBuffer::ID_GENERATOR{};
#endif // COLT_DEBUG
}
