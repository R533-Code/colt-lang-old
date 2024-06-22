#ifndef HG_COLTI_STACK
#define HG_COLTI_STACK

#include "structs/vector.h"
#include "common/types.h"

namespace clt::run
{
  class VMStack
  {
    Vector<QWORD_t> stack;

  public:
    VMStack(u64 reserve = 128) noexcept
      : stack(reserve) {}

    void push(QWORD_t value) noexcept
    {
      stack.push_back(value);
    }

    Option<QWORD_t> pop() noexcept
    {
      if (stack.is_empty())
        return None;
      QWORD_t value = stack.back();
      stack.pop_back();
      return value;
    }
  };
}

#endif // !HG_COLTI_STACK
