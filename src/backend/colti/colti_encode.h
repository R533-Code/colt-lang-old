#ifndef HG_COLTI_ENCODE
#define HG_COLTI_ENCODE

#include "colti_opcodes.h"
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
  };

  /// @brief The result of executing an instruction
  class ExecutionResult
  {
  public:
    /// @brief The possible execution error
    enum class ExecutionError
    {
      /// @brief No errors when executing
      NO_ERRORS,
      /// @brief No more instructions
      END_INST,
      /// @brief Invalid instruction
      INVALID_INST,
      /// @brief Missing operands in the stack
      MISSING_OP
    };

  private:
    /// @brief Stores the ExecutionError
    u8 exec : 2;
    /// @brief Stores the OpError
    u8 payload : 6;

    /// @brief Constructs an execution result
    /// @param exec The execution error
    /// @param payload The operation error (which are only useful when exec == NO_ERRORS)
    constexpr ExecutionResult(ExecutionError exec, OpError payload) noexcept
        : exec(static_cast<u8>(exec))
        , payload(static_cast<u8>(payload))
    {
    }

  public:
    constexpr ExecutionResult()                                  = delete;
    constexpr ExecutionResult(ExecutionResult&&)                 = default;
    constexpr ExecutionResult(const ExecutionResult&)            = default;
    constexpr ExecutionResult& operator=(ExecutionResult&&)      = default;
    constexpr ExecutionResult& operator=(const ExecutionResult&) = default;

    /// @brief Check if the execution result is an error
    /// @return True if an error (END_INST is considered an error)
    constexpr bool is_error() const noexcept
    {
      return static_cast<ExecutionError>(exec) != ExecutionError::NO_ERRORS;
    }
    /// @brief Check if the execution result is a success
    /// @return True if NO_ERRORS
    constexpr bool is_success() const noexcept { return !is_error(); }

    /// @brief Returns the execution result
    /// @return The execution result
    constexpr ExecutionError result() const noexcept
    {
      return static_cast<ExecutionError>(exec);
    }

    /// @brief Returns the warnings associated to the result.
    /// This only makes sense when 'is_success'; these warnings are
    /// useful for the AST.
    /// @return Returns OpError
    constexpr OpError warning() const noexcept
    {
      return static_cast<OpError>(payload);
    }

    /// @brief No errors
    /// @param warn The warning associated with the success
    /// @return ExecutionResult representing a success
    static constexpr ExecutionResult success(
        OpError warn = OpError::NO_ERROR) noexcept
    {
      return ExecutionResult{ExecutionError::NO_ERRORS, warn};
    }
    /// @brief Invalid Instruction
    /// @return ExecutionResult representing an invalid instruction
    static constexpr ExecutionResult invalid_instruction() noexcept
    {
      return ExecutionResult{ExecutionError::INVALID_INST, OpError::NO_ERROR};
    }
    /// @brief Missing Operands from VMStack
    /// @return ExecutionResult representing a missing operand
    static constexpr ExecutionResult missing_operands() noexcept
    {
      return ExecutionResult{ExecutionError::MISSING_OP, OpError::NO_ERROR};
    }
    /// @brief No more instructions
    /// @return ExecutionResult representing the end of instructions
    static constexpr ExecutionResult end_instruction() noexcept
    {
      return ExecutionResult{ExecutionError::END_INST, OpError::NO_ERROR};
    }
  };

  /// @brief Class responsible of executing instructions.
  /// The instructions are decoded and executed in place.
  class VMExecutor
  {
    /// @brief The dispatch table of arithmetic operations.
    /// @warning Matches with OpCodeArithmetic
    static constexpr std::array ArithmeticOperation = {
        &add, &sub, &mul, &div, &mod, nullbinary, &eq, &neq, &le, &ge, &leq, &geq};

    /// @brief Executes an arithmetic instruction
    /// @param ip The instruction pointer
    /// @param stack The stack from which to pop operands
    /// @return The result of executing the instruction
    static ExecutionResult execute_arithmetic(
        InstructionPtr& ip, VMStack& stack) noexcept;
    
    /// @brief Executes a load immediate instruction
    /// @param ip The instruction pointer
    /// @param stack The stack to which to add the immediate
    /// @return The result of executing the instruction 
    static ExecutionResult execute_immediate(
        InstructionPtr& ip, VMStack& stack) noexcept;         

    /// @brief Executes a conversion instruction
    /// @param ip The instruction pointer
    /// @param stack The stack from which to pop operands
    /// @return The result of executing the instruction
    static ExecutionResult execute_conversion(
        InstructionPtr& ip, VMStack& stack) noexcept;

    /// @brief Returns invalid instruction
    /// @param ip The instruction pointer
    /// @param stack 
    /// @return 
    static ExecutionResult execute_invalid(
        InstructionPtr& ip, VMStack& stack) noexcept;

  public:
    static ExecutionResult execute(InstructionPtr& ip, VMStack& stack) noexcept
    {
      // Matches OpCodeFamily
      static constexpr std::array DispatchTable = {
          &execute_arithmetic, &execute_invalid, &execute_immediate, &execute_conversion
      };

      auto value = ip.current();
      if (value.is_none())
        return ExecutionResult::end_instruction();
      const auto family = static_cast<OpCodeFamily>(value.value() >> 5);
      switch_no_default(family)
      {
      case OpCodeFamily::ARITHMETIC:
        return execute_arithmetic(ip, stack);
      case OpCodeFamily::BITWISE:
      case OpCodeFamily::CONVERSION:
        return execute_conversion(ip, stack);
      case OpCodeFamily::BRANCH:
      case OpCodeFamily::IMMEDIATE:
        return execute_immediate(ip, stack);
      case OpCodeFamily::FII:
        break;
      }
    }
  };
} // namespace clt::run

#endif // !HG_COLTI_ENCODE
