#ifndef HG_COLTI_ENCODE
#define HG_COLTI_ENCODE

#include "colti_opcodes.h"
#include "colti_stack.h"

namespace clt::run
{
	class InstructionPtr
	{
		const u8* current_inst;
		const u8* end;

	public:
		constexpr InstructionPtr(const u8* start, const u8* end) noexcept
			: current_inst(start), end(end) {}

		Option<u8> next() noexcept
		{
			if (current_inst == end)
				return None;
			return *current_inst++;
		}
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
			: exec(static_cast<u8>(exec)), payload(static_cast<u8>(payload)) {}

	public:
		constexpr ExecutionResult() = delete;
		constexpr ExecutionResult(ExecutionResult&&) = default;
		constexpr ExecutionResult(const ExecutionResult&) = default;
		constexpr ExecutionResult& operator=(ExecutionResult&&) = default;
		constexpr ExecutionResult& operator=(const ExecutionResult&) = default;

		/// @brief Check if the execution result is an error
		/// @return True if an error (END_INST is considered an error)
		constexpr bool is_error() const noexcept { return static_cast<ExecutionError>(exec) != ExecutionError::NO_ERRORS; }
		/// @brief Check if the execution result is a success
		/// @return True if NO_ERRORS
		constexpr bool is_success() const noexcept { return !is_error(); }

		/// @brief Returns the execution result
		/// @return The execution result
		constexpr ExecutionError result() const noexcept { return static_cast<ExecutionError>(exec); }

		/// @brief Returns the warnings associated to the result.
		/// This only makes sense when 'is_success'; these warnings are
		/// useful for the AST.
		/// @return Returns OpError
		constexpr OpError warning() const noexcept { return static_cast<OpError>(payload); }

		/// @brief No errors
		/// @param warn The warning associated with the success 
		/// @return ExecutionResult representing a success
		static constexpr ExecutionResult success(OpError warn = OpError::NO_ERROR) noexcept
		{
			return ExecutionResult{ ExecutionError::NO_ERRORS, warn };
		}
		/// @brief Invalid Instruction
		/// @return ExecutionResult representing an invalid instruction
		static constexpr ExecutionResult invalid_instruction() noexcept
		{
			return ExecutionResult{ ExecutionError::INVALID_INST, OpError::NO_ERROR };
		}
		/// @brief Missing Operands from VMStack
		/// @return ExecutionResult representing a missing operand
		static constexpr ExecutionResult missing_operands() noexcept
		{
			return ExecutionResult{ ExecutionError::MISSING_OP, OpError::NO_ERROR };
		}
		/// @brief No more instructions
		/// @return ExecutionResult representing the end of instructions
		static constexpr ExecutionResult end_instruction() noexcept
		{
			return ExecutionResult{ ExecutionError::END_INST, OpError::NO_ERROR };
		}
	};

	/// @brief Class responsible of executing instructions.
	/// The instructions are decoded and executed in place.
	struct VMExecutor
	{
		static ExecutionResult execute(InstructionPtr& ip, VMStack& stack) noexcept
		{
			auto value = ip.current();
			if (value.is_none())
				return ExecutionResult::end_instruction();
			const auto family = static_cast<OpCodeFamily>((value.value() & 0b11100000) >> 5);
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
			}
		}

	private:
		static constexpr std::array ArithmeticOperation =
		{
			&add, &sub, &mul, &div, &mod, nullbinary,
			&eq, &neq, &le, &ge, &leq, &geq
		};

		/// [3b:FAMILY = BINARY] [4b:TypeOp] [0]
		/// [0000] [4b:OpCodeBinary]
		static ExecutionResult execute_arithmetic(InstructionPtr& ip, VMStack& stack) noexcept
		{
			using enum OpCodeArithmetic;

			const auto operation = static_cast<OpCodeArithmetic>((ip.current().value() >> 1) & 0b1111);
			// Go to next byte
			auto value = ip.next();
			if (value.is_none())
				return ExecutionResult::invalid_instruction();
			const auto type = static_cast<TypeOp>(value.value() & 0b1111);
			
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
		/// [3b: FAMILY = IMMEDIATE] [3b: Number of Bytes Following] [00]
		/// [8b]{Number of Bytes Following}
		static ExecutionResult execute_immediate(InstructionPtr& ip, VMStack& stack) noexcept
		{
			u64 count = static_cast<u64>((ip.current().value() >> 2) & 0b111) + 1;			
			u64 immediate = 0;

			while (count-- != 0)
			{
				// Go to next byte
				auto value = ip.next();
				if (value.is_none())
					return ExecutionResult::invalid_instruction();
				immediate |= *value;
				immediate <<= 8;
			}
			stack.push(immediate);
		}
		
		/// {[3b:FAMILY=BINARY][4b:TypeOp][0]}{[4b:0][4b:OpCodeBinary]}
		static ExecutionResult execute_conversion(InstructionPtr& ip, VMStack& stack) noexcept
		{

		}
	};
}

#endif // !HG_COLTI_ENCODE
