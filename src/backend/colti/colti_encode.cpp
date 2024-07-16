//#include "colti_encode.h"
//
///// @brief Advances 'ip' to the next instruction, returning if None,
/////        else assigning the value to var_name.
//#define NEXT_OR_RETURN(ip, var_name) \
//  do                                 \
//  {                                  \
//    if (ip.next().is_none())         \
//      return None;                   \
//    var_name = ip.current().value(); \
//  } while (false)
//
//namespace clt::run
//{
//  Option<ArithmeticInst> Decoder::decode_arithmetic(InstructionPtr& ip) noexcept
//  {
//    assert_true(
//        "The current instruction must have the valid family!",
//        decode_current_family(ip).is_value()
//            && *decode_current_family(ip) == OpCodeFamily::ARITHMETIC);
//
//    ON_SCOPE_EXIT
//    {
//      ip.advance();
//    };
//
//    // The instructions are encoded as:
//    // [3b:FAMILY = BINARY] [4b:TypeOp] [0]
//    // [0000] [4b:OpCodeBinary]
//    const auto operation =
//        static_cast<OpCodeArithmetic>((ip.current().value() >> 1) & 0b1111);
//    // Go to next byte
//    u8 value;
//    NEXT_OR_RETURN(ip, value);
//    const auto type = static_cast<TypeOp>(value & 0b1111);
//
//    if ((u8)operation < reflect<OpCodeArithmetic>::count()
//        && (u8)type < reflect<TypeOp>::count())
//      return ArithmeticInst{operation, type};
//    return None;
//  }
//
//  Option<BitwiseInst> Decoder::decode_bitwise(InstructionPtr& ip) noexcept
//  {
//    assert_true(
//        "The current instruction must have the valid family!",
//        decode_current_family(ip).is_value()
//            && *decode_current_family(ip) == OpCodeFamily::BITWISE);
//
//    ON_SCOPE_EXIT
//    {
//      ip.advance();
//    };
//
//    // The instructions are encoded as:
//    // [3b:FAMILY = BITWISE] [3b: OpCodeBitwise] [00]
//    // [6b: Number of bits] [00]
//    const auto operation =
//        static_cast<OpCodeBitwise>((ip.current().value() >> 2) & 0b111);
//    // Go to next byte
//    u8 value;
//    NEXT_OR_RETURN(ip, value);
//    const auto size = static_cast<u8>(value >> 2);
//    // The size is always valid as it must represent 0-63,
//    // which can exactly be represented using 6 bits.
//
//    if ((u8)operation < reflect<OpCodeArithmetic>::count())
//      return BitwiseInst{operation, size};
//    return None;
//  }
//
//  Option<ConvertInst> Decoder::decode_conversion(InstructionPtr& ip) noexcept
//  {
//    assert_true(
//        "The current instruction must have the valid family!",
//        decode_current_family(ip).is_value()
//            && *decode_current_family(ip) == OpCodeFamily::CONVERSION);
//
//    ON_SCOPE_EXIT
//    {
//      ip.advance();
//    };
//
//    // The instructions are encoded as:
//    // [3b: FAMILY = CONVERSION] [0 0000]
//    // [4b: From TypeOp] [4b: To TypeOp]
//
//    // Go to next byte
//    u8 value;
//    NEXT_OR_RETURN(ip, value);
//    const auto to   = static_cast<TypeOp>(value & 0b1111);
//    const auto from = static_cast<TypeOp>(value >> 4);
//    if ((u8)to < reflect<TypeOp>::count() && (u8)from < reflect<TypeOp>::count())
//      return ConvertInst{from, to};
//    return None;
//  }
//
//  Option<ImmediateInst> Decoder::decode_immediate(InstructionPtr& ip) noexcept
//  {
//    assert_true(
//        "The current instruction must have the valid family!",
//        decode_current_family(ip).is_value()
//            && *decode_current_family(ip) == OpCodeFamily::IMMEDIATE);
//
//    ON_SCOPE_EXIT
//    {
//      ip.advance();
//    };
//
//    // The instructions are encoded as:
//    /// [3b: FAMILY = IMMEDIATE] [3b: Number of Bytes Following] [00]
//    /// [8b]{Number of Bytes Following - 1}
//
//    // The count of bytes following
//    u64 count = static_cast<u64>((ip.current().value() >> 2) & 0b111) + 1;
//    // The immediate to push
//    u64 immediate = 0;
//
//    while (count-- != 0)
//    {
//      // Go to next byte
//      u8 value;
//      NEXT_OR_RETURN(ip, value);
//      immediate |= value;
//      immediate <<= 8;
//    }
//    return ImmediateInst{immediate};
//  }
//
//  Option<BranchInst> Decoder::decode_branch(InstructionPtr& ip) noexcept
//  {
//    assert_true(
//        "The current instruction must have the valid family!",
//        decode_current_family(ip).is_value()
//            && *decode_current_family(ip) == OpCodeFamily::BRANCH);
//
//    ON_SCOPE_EXIT
//    {
//      ip.advance();
//    };
//
//    using enum OpCodeBranch;
//
//    // The instructions are encoded as:
//    // [3b: FAMILY = BRANCH] [3b: OpCodeBranch] [00]
//    // [8b]*7 (if not _calli or _ret)
//
//    const auto opcode =
//        static_cast<OpCodeBranch>((ip.current().value() >> 2) & 0b111);
//    if (opcode == _calli || opcode == _ret)
//      return BranchInst{opcode, 0};
//    // The count of bytes following
//    u64 count     = 8;
//    i64 immediate = 0;
//    while (count-- != 0)
//    {
//      // Go to next byte
//      u8 value;
//      NEXT_OR_RETURN(ip, value);
//      immediate |= value;
//      immediate <<= 8;
//    }
//    if ((u8)opcode < reflect<OpCodeBranch>::count())
//      return BranchInst{opcode, immediate};
//    return None;
//  }
//
//  Option<OpCodeFamily> Decoder::decode_current_family(
//      const InstructionPtr& ip) noexcept
//  {
//    auto value = ip.current();
//    if (value.is_none())
//      return None;
//    const auto family = static_cast<OpCodeFamily>(value.value() >> 5);
//    if ((u8)family < reflect<OpCodeFamily>::count())
//      return None;
//    return family;
//  }
//
//  ExecutionResult VMExecutor::execute_arithmetic(
//      InstructionPtr& ip, VMStack& stack) noexcept
//  {
//    using enum OpCodeArithmetic;
//
//    // Decode the instruction
//    auto decode = Decoder::decode_arithmetic(ip);
//    if (decode.is_none())
//      return ExecutionResult::invalid_instruction();
//
//    // Negate is the only arithmetic operation that is unary,
//    // so we handle this case alone
//    if (decode->is_unary())
//    {
//      auto op1 = stack.pop();
//      if (op1.is_none())
//        return ExecutionResult::missing_operands();
//      auto [result, warn] =
//          ArithmeticUnOp[(u8)decode->arithmetic - (u8)_neg](*op1, decode->type);
//      if (warn == INVALID_OP)
//        return ExecutionResult::invalid_instruction();
//      stack.push(result);
//      return ExecutionResult::success(warn);
//    }
//
//    // Handles the binary arithmetic operations.
//    auto op1 = stack.pop();
//    auto op2 = stack.pop();
//    if (op1.is_none() || op2.is_none())
//      return ExecutionResult::missing_operands();
//
//    auto [value, warn] =
//        ArithmeticBinOp[(u8)decode->arithmetic](*op1, *op2, decode->type);
//    if (warn == INVALID_OP)
//      return ExecutionResult::invalid_instruction();
//    // Push the result on the stack
//    stack.push(value);
//    return ExecutionResult::success(warn);
//  }
//
//  ExecutionResult VMExecutor::execute_bitwise(
//      InstructionPtr& ip, VMStack& stack) noexcept
//  {
//    using enum OpCodeBitwise;
//
//    // Decode the instruction
//    auto decode = Decoder::decode_bitwise(ip);
//    if (decode.is_none())
//      return ExecutionResult::invalid_instruction();
//    // Negate is the only arithmetic operation that is unary,
//    // so we handle this case alone
//    if (decode->is_unary())
//    {
//      auto op1 = stack.pop();
//      if (op1.is_none())
//        return ExecutionResult::missing_operands();
//      auto [result, warn] =
//          BitwiseUnOp[(u8)decode->bitwise - (u8)_not](*op1, decode->bits);
//      if (warn == INVALID_OP)
//        return ExecutionResult::invalid_instruction();
//      stack.push(result);
//      return ExecutionResult::success(warn);
//    }
//
//    // Handles the binary arithmetic operations.
//    auto op1 = stack.pop();
//    auto op2 = stack.pop();
//    if (op1.is_none() || op2.is_none())
//      return ExecutionResult::missing_operands();
//
//    auto [value, warn] = BitwiseBinOp[(u8)decode->bitwise](*op1, *op2, decode->bits);
//
//    if (warn == INVALID_OP)
//      return ExecutionResult::invalid_instruction();
//    // Push the result on the stack
//    stack.push(value);
//    return ExecutionResult::success(warn);
//  }
//
//  ExecutionResult VMExecutor::execute_immediate(
//      InstructionPtr& ip, VMStack& stack) noexcept
//  {
//    // Decode the instruction
//    auto decode = Decoder::decode_immediate(ip);
//    if (decode.is_none())
//      return ExecutionResult::invalid_instruction();
//    stack.push(decode->immediate);
//  }
//
//  ExecutionResult VMExecutor::execute_branch(
//      InstructionPtr& ip, VMStack& stack) noexcept
//  {
//    return ExecutionResult();
//  }
//
//  ExecutionResult VMExecutor::execute_conversion(
//      InstructionPtr& ip, VMStack& stack) noexcept
//  {
//    auto conv_opt = Decoder::decode_conversion(ip);
//    if (conv_opt.is_none())
//      return ExecutionResult::invalid_instruction();
//
//    auto op1 = stack.pop();
//    if (op1.is_none())
//      return ExecutionResult::missing_operands();
//  }
//
//  ExecutionResult VMExecutor::execute_ffi(
//      InstructionPtr& ip, VMStack& stack) noexcept
//  {
//    return ExecutionResult();
//  }
//
//  ExecutionResult VMExecutor::execute_invalid(
//      InstructionPtr& ip, VMStack& stack) noexcept
//  {
//    // Consume current byte
//    ip.next();
//    return ExecutionResult::invalid_instruction();
//  }
//
//  ExecutionResult VMExecutor::execute(InstructionPtr& ip, VMStack& stack) noexcept
//  {
//    // Matches OpCodeFamily
//    static constexpr std::array DispatchTable = {
//        &execute_arithmetic, &execute_bitwise, &execute_immediate,
//        &execute_conversion, &execute_branch,  &execute_ffi};
//
//    if (auto family_opt = Decoder::decode_current_family(ip); family_opt.is_value())
//      return DispatchTable[(u8)*family_opt](ip, stack);
//    // Consume current erroneous byte
//    ip.next();
//    return ExecutionResult::invalid_instruction();
//  }
//} // namespace clt::run
//
//#undef NEXT_OR_RETURN