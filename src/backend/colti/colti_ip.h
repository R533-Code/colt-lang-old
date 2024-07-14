#ifndef HG_COLTI_IP
#define HG_COLTI_IP

#include "colti_stack.h"

namespace clt::run
{
  /// @brief Represents an instruction pointer of the ColtVM.
  /// As the ColtVM is not striving for performance but
  /// rather safety and detection of errors, all operation
  /// done on the instruction pointer are checked.
  class InstructionPtr
  {
    /// @brief The current instruction being executed
    const u8* current_inst;
    /// @brief The start of the code section
    const u8* const begin;
    /// @brief The end of the code section
    const u8* const end;

  public:
    /// @brief Constructor
    /// @param current The current instruction to execute
    /// @param begin The start of the code section
    /// @param end The end of the code section
    constexpr InstructionPtr(
        const u8* current, const u8* const begin, const u8* const end) noexcept
        : current_inst(current)
        , begin(begin)
        , end(end)
    {
    }

    /// @brief Returns the next instruction
    /// @return The next instruction or None if no more instructions exists
    Option<u8> next() noexcept
    {
      if (current_inst == end)
        return None;
      return *current_inst++;
    }

    /// @brief Returns the current instruction
    /// @return The current instruction of None if no more instructions exists
    Option<u8> current() const noexcept
    {
      if (current_inst == end)
        return None;
      return *current_inst;
    }

    /// @brief Advances to the next instruction
    void advance() noexcept
    {
      if (current_inst == end)
        current_inst++;
    }

    /// @brief Advances the instruction pointer by an offset
    /// @param offset The offset to add to the instruction pointer
    /// @return Success if the resulting instruction pointer is valid
    ErrorFlag add(i64 offset) noexcept
    {
      auto temp = current_inst + offset;
      if (begin <= temp && temp < end)
      {
        current_inst = temp;
        return ErrorFlag::success();
      }
      return ErrorFlag::error();
    }

    /// @brief Sets the instruction pointer to 'offset'
    /// @param offset The offset (in bytes from the beginning of the code section)
    /// @return Success if the resulting instruction pointer is valid
    ErrorFlag set(u64 offset) noexcept
    {
      auto temp = begin + offset;
      if (temp < end)
      {
        current_inst = temp;
        return ErrorFlag::success();
      }
      return ErrorFlag::error();
    }
  };
}

#endif // !HG_COLTI_IP
