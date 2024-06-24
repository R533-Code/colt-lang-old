#include "colti_encode.h"

/// @brief Advances 'ip' to the next instruction, returning if None,
///        else assigning the value to var_name.
#define NEXT_OR_RETURN(ip, var_name)                 \
  do                                                 \
  {                                                  \
    if (ip.next().is_none())                         \
      return ExecutionResult::invalid_instruction(); \
    var_name = ip.current().value();                 \
  } while (false)

namespace clt::run
{
  ExecutionResult VMExecutor::execute_arithmetic(
      InstructionPtr& ip, VMStack& stack) noexcept
  {
    // The instructions are encoded as:
    // [3b:FAMILY = BINARY] [4b:TypeOp] [0]
    // [0000] [4b:OpCodeBinary]

    using enum OpCodeArithmetic;

    const auto operation =
        static_cast<OpCodeArithmetic>((ip.current().value() >> 1) & 0b1111);
    // Go to next byte
    u8 value;
    NEXT_OR_RETURN(ip, value);
    const auto type = static_cast<TypeOp>(value & 0b1111);

    // Negate is the only arithmetic operation that is unary,
    // so we handle this case alone
    if (operation == _neg)
    {
      auto op1 = stack.pop();
      if (op1.is_none())
        return ExecutionResult::missing_operands();
      auto [result, warn] = neg(*op1, type);
      if (warn == INVALID_OP)
        return ExecutionResult::invalid_instruction();
      stack.push(result);
      return ExecutionResult::success(warn);
    }

    // Handles the binary arithmetic operations.
    auto op1 = stack.pop();
    auto op2 = stack.pop();
    if (op1.is_none() || op2.is_none())
      return ExecutionResult::missing_operands();

    ResultQWORD op;
    if ((u8)operation < ArithmeticOperation.size())
      op = ArithmeticOperation[(u8)operation](*op1, *op2, type);
    else
      return ExecutionResult::invalid_instruction();

    if (op.second == INVALID_OP)
      return ExecutionResult::invalid_instruction();
    // Push the result on the stack
    stack.push(op.first);
    return ExecutionResult::success(op.second);
  }

  ExecutionResult VMExecutor::execute_immediate(
      InstructionPtr& ip, VMStack& stack) noexcept
  {
    // The instructions are encoded as:
    /// [3b: FAMILY = IMMEDIATE] [3b: Number of Bytes Following] [00]
    /// [8b]{Number of Bytes Following}

    // The count of bytes following
    u64 count = static_cast<u64>((ip.current().value() >> 2) & 0b111) + 1;
    // The immediate to push
    u64 immediate = 0;

    while (count-- != 0)
    {
      // Go to next byte
      u8 value;
      NEXT_OR_RETURN(ip, value);
      immediate |= value;
      immediate <<= 8;
    }
    stack.push(immediate);
  }

  ExecutionResult VMExecutor::execute_conversion(
      InstructionPtr& ip, VMStack& stack) noexcept
  {
    // The instructions are encoded as:
    // [3b: FAMILY = CONVERSION] [0 0000]
    // [4b: TypeOp] [4b: TypeOp]

    // Go to next byte
    u8 value;
    NEXT_OR_RETURN(ip, value);
    //auto
  }
} // namespace clt::run

#undef NEXT_OR_RETURN