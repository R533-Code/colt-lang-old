//#ifndef HG_COLTI_ENCODE
//#define HG_COLTI_ENCODE
//
//#include "colti_opcodes.h"
//#include "colti_stack.h"
//#include "colti_ip.h"
//
//namespace clt::run
//{
//  /// @brief The result of executing an instruction
//  class ExecutionResult
//  {
//  public:
//    /// @brief The possible execution error
//    enum class ExecutionError
//    {
//      /// @brief No errors when executing
//      NO_ERRORS,
//      /// @brief No more instructions
//      END_INST,
//      /// @brief Invalid instruction
//      INVALID_INST,
//      /// @brief Missing operands in the stack
//      MISSING_OP
//    };
//
//  private:
//    /// @brief Stores the ExecutionError
//    u8 exec : 2;
//    /// @brief Stores the OpError
//    u8 payload : 6;
//
//    /// @brief Constructs an execution result
//    /// @param exec The execution error
//    /// @param payload The operation error (which are only useful when exec == NO_ERRORS)
//    constexpr ExecutionResult(ExecutionError exec, OpError payload) noexcept
//        : exec(static_cast<u8>(exec))
//        , payload(static_cast<u8>(payload))
//    {
//    }
//
//  public:
//    constexpr ExecutionResult()                                  = delete;
//    constexpr ExecutionResult(ExecutionResult&&)                 = default;
//    constexpr ExecutionResult(const ExecutionResult&)            = default;
//    constexpr ExecutionResult& operator=(ExecutionResult&&)      = default;
//    constexpr ExecutionResult& operator=(const ExecutionResult&) = default;
//
//    /// @brief Check if the execution result is an error
//    /// @return True if an error (END_INST is considered an error)
//    constexpr bool is_error() const noexcept
//    {
//      return static_cast<ExecutionError>(exec) != ExecutionError::NO_ERRORS;
//    }
//    /// @brief Check if the execution result is a success
//    /// @return True if NO_ERRORS
//    constexpr bool is_success() const noexcept { return !is_error(); }
//
//    /// @brief Returns the execution result
//    /// @return The execution result
//    constexpr ExecutionError result() const noexcept
//    {
//      return static_cast<ExecutionError>(exec);
//    }
//
//    /// @brief Returns the warnings associated to the result.
//    /// This only makes sense when 'is_success'; these warnings are
//    /// useful for the AST.
//    /// @return Returns OpError
//    constexpr OpError warning() const noexcept
//    {
//      return static_cast<OpError>(payload);
//    }
//
//    /// @brief No errors
//    /// @param warn The warning associated with the success
//    /// @return ExecutionResult representing a success
//    static constexpr ExecutionResult success(
//        OpError warn = OpError::NO_ERROR) noexcept
//    {
//      return ExecutionResult{ExecutionError::NO_ERRORS, warn};
//    }
//    /// @brief Invalid Instruction
//    /// @return ExecutionResult representing an invalid instruction
//    static constexpr ExecutionResult invalid_instruction() noexcept
//    {
//      return ExecutionResult{ExecutionError::INVALID_INST, OpError::NO_ERROR};
//    }
//    /// @brief Missing Operands from VMStack
//    /// @return ExecutionResult representing a missing operand
//    static constexpr ExecutionResult missing_operands() noexcept
//    {
//      return ExecutionResult{ExecutionError::MISSING_OP, OpError::NO_ERROR};
//    }
//    /// @brief No more instructions
//    /// @return ExecutionResult representing the end of instructions
//    static constexpr ExecutionResult end_instruction() noexcept
//    {
//      return ExecutionResult{ExecutionError::END_INST, OpError::NO_ERROR};
//    }
//  };
//
//  /// @brief Class responsible for decoding instructions
//  struct Decoder
//  {
//    /// @brief Decodes an arithmetic instruction
//    /// @param ip The instruction pointer
//    /// @return None on error, or the decoded instruction
//    static Option<ArithmeticInst> decode_arithmetic(InstructionPtr& ip) noexcept;
//    /// @brief Decodes a bitwise instruction
//    /// @param ip The instruction pointer
//    /// @return None on error, or the decoded instruction
//    static Option<BitwiseInst> decode_bitwise(InstructionPtr& ip) noexcept;
//    /// @brief Decodes a conversion instruction
//    /// @param ip The instruction pointer
//    /// @return None on error, or the decoded instruction
//    static Option<ConvertInst> decode_conversion(InstructionPtr& ip) noexcept;
//    /// @brief Decodes an immediate instruction
//    /// @param ip The instruction pointer
//    /// @return None on error, or the decoded instruction
//    static Option<ImmediateInst> decode_immediate(InstructionPtr& ip) noexcept;
//    /// @brief Decodes a branch instruction
//    /// @param ip The instruction pointer
//    /// @return None on error, or the decoded instruction
//    static Option<BranchInst> decode_branch(InstructionPtr& ip) noexcept;
//
//    /// @brief Decodes the current family.
//    /// This function does NOT advance the instruction pointer.
//    /// Usually the caller should advance the instruction pointer
//    /// if None was returned.
//    /// @param ip The instruction pointer
//    /// @return The OpCodeFamily or None on errors
//    static Option<OpCodeFamily> decode_current_family(
//        const InstructionPtr& ip) noexcept;
//  };
//
//  /// @brief Class responsible of executing instructions.
//  /// The instructions are decoded and executed in place.
//  class VMExecutor
//  {
//    /// @brief The dispatch table of binary arithmetic operations.
//    /// @warning Matches with OpCodeArithmetic
//    static constexpr std::array ArithmeticBinOp = {
//        &add, &sub, &mul, &div, &mod, &eq, &neq, &le, &ge, &leq, &geq};
//
//    /// @brief The dispatch table of unary arithmetic operations.
//    /// @warning Matches with OpCodeArithmetic
//    static constexpr std::array ArithmeticUnOp = {&neg};
//
//    /// @brief The dispatch table of bitwise binary operations.
//    /// @warning Matches with OpCodeBitwise
//    static constexpr std::array BitwiseBinOp = {
//        &bit_and, &bit_or, &bit_xor, &lsr, &lsl, &asr};
//
//    /// @brief The dispatch table of unary bitwise operations.
//    /// @warning Matches with OpCodeBitwise
//    static constexpr std::array BitwiseUnOp = {&bit_not};
//
//    /// @brief Executes an arithmetic instruction
//    /// @param ip The instruction pointer
//    /// @param stack The stack from which to pop operands
//    /// @return The result of executing the instruction
//    static ExecutionResult execute_arithmetic(
//        InstructionPtr& ip, VMStack& stack) noexcept;
//
//    /// @brief Executes an bitwise instruction
//    /// @param ip The instruction pointer
//    /// @param stack The stack from which to pop operands
//    /// @return The result of executing the instruction
//    static ExecutionResult execute_bitwise(
//        InstructionPtr& ip, VMStack& stack) noexcept;
//
//    /// @brief Executes a load immediate instruction
//    /// @param ip The instruction pointer
//    /// @param stack The stack to which to add the immediate
//    /// @return The result of executing the instruction
//    static ExecutionResult execute_immediate(
//        InstructionPtr& ip, VMStack& stack) noexcept;
//
//    /// @brief Executes a branch instruction
//    /// @param ip The instruction pointer
//    /// @param stack The stack to which to add the immediate
//    /// @return The result of executing the instruction
//    static ExecutionResult execute_branch(
//        InstructionPtr& ip, VMStack& stack) noexcept;
//
//    /// @brief Executes a conversion instruction
//    /// @param ip The instruction pointer
//    /// @param stack The stack from which to pop operands
//    /// @return The result of executing the instruction
//    static ExecutionResult execute_conversion(
//        InstructionPtr& ip, VMStack& stack) noexcept;
//
//    /// @brief Executes an ffi instruction
//    /// @param ip The instruction pointer
//    /// @param stack The stack from which to pop operands
//    /// @return The result of executing the instruction
//    static ExecutionResult execute_ffi(InstructionPtr& ip, VMStack& stack) noexcept;
//
//    /// @brief Returns invalid instruction, consuming the current byte
//    /// @param ip The instruction pointer
//    /// @param stack The stack (unused)
//    /// @return The result of executing the instruction
//    static ExecutionResult execute_invalid(
//        InstructionPtr& ip, VMStack& stack) noexcept;
//
//  public:
//    /// @brief Executes the next instruction.
//    /// The execution result can contain additional information:
//    /// when 'is_success', the instruction could have generated warnings
//    /// (OpError).
//    /// @param ip The instruction pointer to advance
//    /// @param stack The stack used for intermediate results
//    /// @return The execution result
//    static ExecutionResult execute(InstructionPtr& ip, VMStack& stack) noexcept;
//  };
//} // namespace clt::run
//
//#endif // !HG_COLTI_ENCODE
