/*****************************************************************//**
 * @file   colt_operators.h
 * @brief  Contains the possible colt operators.
 * 
 * @author RPC
 * @date   February 2024
 *********************************************************************/
#ifndef HG_COLT_OPERATORS
#define HG_COLT_OPERATORS

#include <util/colt_pch.h>
#include "colt_lexeme.h"

DECLARE_ENUM_WITH_TYPE(u8, clt::lng, UnaryOp,
	/// @brief ++...
	OP_INC,
	/// @brief --...
	OP_DEC,
  /// @brief -...
  OP_NEGATE,
  /// @brief !...
  OP_BOOL_NOT,
  /// @brief ~...
  OP_BIT_NOT
);

DECLARE_ENUM_WITH_TYPE(u8, clt::lng, BinaryOp,
	/*********** ARITHMETIC ***********/

	/// @brief +
	OP_SUM,
	/// @brief -
	OP_SUB,
	/// @brief *
	OP_MUL,
	/// @brief /
	OP_DIV,
	/// @brief %
	OP_MOD,

	/*********** BITWISE ***********/

	/// @brief &
	OP_BIT_AND,
	/// @brief |
	OP_BIT_OR,
	/// @brief ^
	OP_BIT_XOR,
	/// @brief <<
	OP_BIT_LSHIFT,
	/// @brief >>
	OP_BIT_RSHIFT,

	/*********** BOOLEANS ***********/

	/// @brief &&
	OP_BOOL_AND,
	/// @brief ||
	OP_BOOL_OR,
	/// @brief <
	OP_LESS,
	/// @brief <=
	OP_LESS_EQUAL,
	/// @brief >
	OP_GREAT,
	/// @brief >=
	OP_GREAT_EQUAL,
	/// @brief !=
	OP_NOT_EQUAL,
	/// @brief ==
	OP_EQUAL,
);

DECLARE_ENUM_WITH_TYPE(u8, clt::lng, OpFamily,
	ARITHMETIC,
	BIT_LOGIC,
	BOOL_LOGIC,
	COMPARISON
);

namespace clt::lng
{	
	namespace details
	{
		/// @brief The operator precedence table
		constexpr std::array<u8, 18> OPERATOR_PRECEDENCE_TABLE =
		{
			12, 12, 13, 13, 13, // + - * / %
			10, 10, 10, 11, 11,  // & | ^ << >>
			3, 2,  // '&&' '||'
			7, 7, 7, 7, 6, 6 // < <= > >= != ==
		};
	}

	/// @brief Returns the precedence of a Lexeme
	/// @param tkn The token whose precedence to return
	/// @return The precedence of the token
	constexpr u8 OpPrecedence(BinaryOp op) noexcept
	{
		return op <= BinaryOp::OP_EQUAL ?
			details::OPERATOR_PRECEDENCE_TABLE[static_cast<u8>(op)] : 0;
	}

	/// @brief Returns the precedence of a Lexeme
	/// @param tkn The token whose precedence to return
	/// @return The precedence of the token
	constexpr u8 OpPrecedence(Lexeme tkn) noexcept
	{
		// break Pratt's parsing
		return tkn <= Lexeme::TKN_EQUAL_EQUAL ?
			details::OPERATOR_PRECEDENCE_TABLE[static_cast<u8>(tkn)] : 0;
	}

	/// @brief Returns the family of a binary operator
	/// @param op The operator whose family to return
	/// @return The family of 'op'
	constexpr OpFamily FamilyOf(BinaryOp op) noexcept
	{
		using enum OpFamily;
		using enum BinaryOp;
		
		if (OP_SUM <= op && op <= OP_MOD)
			return ARITHMETIC;
		if (OP_BIT_AND <= op && op <= OP_BIT_RSHIFT)
			return BIT_LOGIC;
		if (op == OP_BOOL_AND || op == OP_BOOL_OR)
			return BOOL_LOGIC;
		if (OP_LESS <= op && op <= OP_EQUAL)
			return COMPARISON;
		unreachable("Unknow operator!");
	}

	/// @brief Converts a valid unary token to a UnaryOp
	/// @param tkn The Lexeme to convert
	/// @return Assertion failure or a valid UnaryOp
	constexpr UnaryOp TokenToUnary(Lexeme tkn) noexcept
	{
		using enum Lexeme;

		switch_no_default (tkn)
		{
		case TKN_PLUS_PLUS:
			return UnaryOp::OP_INC;
		case TKN_MINUS_MINUS:
			return UnaryOp::OP_DEC;
		case TKN_MINUS:
			return UnaryOp::OP_NEGATE;
		case TKN_EXCLAM:
			return UnaryOp::OP_BOOL_NOT;
		case TKN_TILDE:
			return UnaryOp::OP_BIT_NOT;
		}
	}

	/// @brief Converts a valid binary token to a BinaryOp
	/// @param tkn The Lexeme to convert
	/// @return Assertion failure or a valid BinaryOp
	constexpr BinaryOp TokenToBinary(Lexeme tkn) noexcept
	{
		assert_true("Expected a valid binary token!", isBinaryToken(tkn));
		return static_cast<BinaryOp>(tkn);
	}  

	/// @brief Converts a binary operator to its string equivalent
	/// @param op The operator
	/// @return The string representing the operator
	constexpr const char* toStr(lng::BinaryOp op) noexcept
	{
		using enum lng::BinaryOp;

		switch_no_default (op)
		{
		case OP_SUM:
			return "+";
		case OP_SUB:
			return "-";
		case OP_MUL:
			return "*";
		case OP_DIV:
			return "/";
		case OP_MOD:
			return "%";
		case OP_BIT_AND:
			return "&";
		case OP_BIT_OR:
			return "|";
		case OP_BIT_XOR:
			return "^";
		case OP_BIT_LSHIFT:
			return "<<";
		case OP_BIT_RSHIFT:
			return ">>";
		case OP_BOOL_AND:
			return "&&";
		case OP_BOOL_OR:
			return "||";
		case OP_LESS:
			return "<";
		case OP_LESS_EQUAL:
			return "<=";
		case OP_GREAT:
			return ">";
		case OP_GREAT_EQUAL:
			return ">=";
		case OP_NOT_EQUAL:
			return "!=";
		case OP_EQUAL:
			return "==";
		}
	}

	/// @brief Converts a unary operator to its string equivalent
	/// @param op The unary operator
	/// @return The string representing the operator
	constexpr const char* toStr(lng::UnaryOp op) noexcept
	{
		using enum lng::UnaryOp;

		switch_no_default (op)
		{
		case OP_INC:
			return "++";
		case OP_DEC:
			return "--";
		case OP_NEGATE:
			return "-";
		case OP_BOOL_NOT:
			return "!";
		case OP_BIT_NOT:
			return "~";
		}
	}
}

#endif //!HG_COLT_OPERATORS