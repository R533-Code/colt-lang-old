/*****************************************************************//**
 * @file   colt_token.h
 * @brief  Contains an enum for all valid tokens (representing a lexeme)
 *				 of the Colt language.
 * 
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_TOKEN
#define HG_COLT_TOKEN

#include "util/assertions.h"
#include "util/types.h"

namespace clt::lng
{
	/// @brief Represents the lexeme of the Colt language.
	/// To optimize switch and category checks, the Token
	/// is a bit complex in its layout and order.
	/// Comments explains how to add new Tokens without
	/// causing problems with `is*Token` functions.
	enum class Token
		: u8
	{
		/********* BEGINNING OF BINARY OPERATORS *******/
		/// While not all symbols in this section are binary
		/// operators, some are considered as such to simplify
		/// Pratt's Parsing in the AST.

		/// The TKN_PLUS is used as a delimiter for unary
		/// operators: do not add anything before it.

		/// @brief +
		TKN_PLUS,
		/// @brief -
		TKN_MINUS,
		/// @brief *
		TKN_STAR,

		/// The TKN_SLASH is used as a delimiter for unary
		/// operators: do not add anything before it.

		/// @brief /
		TKN_SLASH,
		/// @brief %
		TKN_PERCENT,
		/// @brief &
		TKN_AND,
		/// @brief |
		TKN_OR,
		/// @brief ^
		TKN_CARET,
		/// @brief <<
		TKN_LESS_LESS,
		/// @brief >>
		TKN_GREAT_GREAT,

		/// The TKN_GREAT_GREAT is used as a delimiter for comparison
		/// operators: do not add non comparison operators tokens
		/// after it.

		/// @brief &&
		TKN_AND_AND,
		/// @brief ||
		TKN_OR_OR,

		/// @brief <
		TKN_LESS,
		/// @brief <=
		TKN_LESS_EQUAL,

		/// @brief >
		TKN_GREAT,
		/// @brief >=
		TKN_GREAT_EQUAL,

		/// @brief !=
		TKN_BANG_EQUAL,
		/// @brief ==
		TKN_EQUAL_EQUAL,

		/********* BEGINNING OF ASSIGNMENT OPERATORS *******/

		/// @brief =
		TKN_EQUAL,
		/// @brief +=
		TKN_PLUS_EQUAL,
		/// @brief -=
		TKN_MINUS_EQUAL,
		/// @brief *=
		TKN_STAR_EQUAL,
		/// @brief /=
		TKN_SLASH_EQUAL,
		/// @brief %=
		TKN_PERCENT_EQUAL,
		/// @brief &=
		TKN_AND_EQUAL,
		/// @brief |=
		TKN_OR_EQUAL,
		/// @brief ^=
		TKN_CARET_EQUAL,
		/// @brief <<=
		TKN_LESS_LESS_EQUAL,
		/// @brief >>=
		TKN_GREAT_GREAT_EQUAL,

		/********* END OF ASSIGNMENT OPERATORS *******/
		/********* END OF BINARY OPERATORS *******/

		/// @brief ,
		TKN_COMMA,
		/// @brief ;
		TKN_SEMICOLON,
		/// @brief end of file
		TKN_EOF,
		/// @brief error detected lexeme
		TKN_ERROR,
		/// @brief )
		TKN_RIGHT_PAREN,
		/// @brief (
		TKN_LEFT_PAREN,


		/// @brief :
		TKN_COLON,
		/// @brief ::
		TKN_COLON_COLON,
		/// @brief }
		TKN_RIGHT_CURLY,
		/// @brief {
		TKN_LEFT_CURLY,

		/// @brief ->
		TKN_MINUS_GREAT,
		/// @brief =>
		TKN_EQUAL_GREAT,

		/// @brief ++
		TKN_PLUS_PLUS,
		/// @brief --
		TKN_MINUS_MINUS,
		/// @brief ~
		TKN_TILDE,
		/// @brief !
		TKN_BANG,

		/// @brief [
		TKN_LEFT_SQUARE,
		/// @brief ]
		TKN_RIGHT_SQUARE,

		/********* BEGINNING OF LITERAL TOKENS *******/

		/// @brief true/false
		TKN_BOOL_L,
		/// @brief '.'
		TKN_CHAR_L,
		/// @brief NUMu8
		TKN_U8_L,
		/// @brief NUMu16
		TKN_U16_L,
		/// @brief NUMu32
		TKN_U32_L,
		/// @brief NUMu64
		TKN_U64_L,
		/// @brief NUMi8
		TKN_I8_L,
		/// @brief NUMi16
		TKN_I16_L,
		/// @brief NUMi32
		TKN_I32_L,
		/// @brief NUMi64
		TKN_I64_L,
		/// @brief REALf
		TKN_FLOAT_L,
		/// @brief REALd
		TKN_DOUBLE_L,
		/// @brief "..."
		TKN_STRING_L,

		/********* END OF LITERAL TOKENS *******/
		
		/********* LOOPS AND CONDITIONALS *******/

		/// @brief if
		TKN_KEYWORD_IF,
		/// @brief elif
		TKN_KEYWORD_ELIF,
		/// @brief else
		TKN_KEYWORD_ELSE,

		/// @brief for
		TKN_KEYWORD_FOR,
		/// @brief while
		TKN_KEYWORD_WHILE,
		/// @brief break
		TKN_KEYWORD_BREAK,
		/// @brief continue
		TKN_KEYWORD_CONTINUE,

		/********* VARIABLE DECLARATIONS *******/

		/// @brief var
		TKN_KEYWORD_VAR,
		/// @brief mut
		TKN_KEYWORD_MUT,
		/// @brief global
		TKN_KEYWORD_GLOBAL,

		/********* BEGINNING OF BUILTIN TYPES *******/

		/// @brief void
		TKN_KEYWORD_VOID,
		/// @brief bool
		TKN_KEYWORD_BOOL,
		/// @brief char
		TKN_KEYWORD_CHAR,
		/// @brief u8
		TKN_KEYWORD_U8,
		/// @brief u16
		TKN_KEYWORD_U16,
		/// @brief u32
		TKN_KEYWORD_U32,
		/// @brief u64
		TKN_KEYWORD_U64,
		/// @brief i8
		TKN_KEYWORD_I8,
		/// @brief i16
		TKN_KEYWORD_I16,
		/// @brief i32
		TKN_KEYWORD_I32,
		/// @brief i64
		TKN_KEYWORD_I64,
		/// @brief f32
		TKN_KEYWORD_FLOAT,
		/// @brief f64
		TKN_KEYWORD_DOUBLE,
		/// @brief BYTE
		TKN_KEYWORD_BYTE,
		/// @brief WORD
		TKN_KEYWORD_WORD,
		/// @brief DWORD
		TKN_KEYWORD_DWORD,
		/// @brief QWORD
		TKN_KEYWORD_QWORD,		
		/// @brief ptr
		TKN_KEYWORD_PTR,
		/// @brief mut_ptr
		TKN_KEYWORD_MUT_PTR,
		/// @brief opaque
		TKN_KEYWORD_OPAQUE,

		/********* END OF BUILTIN TYPES *******/

		/// @brief fn
		TKN_KEYWORD_FN,
		/// @brief return
		TKN_KEYWORD_RETURN,
		/// @brief extern
		TKN_KEYWORD_EXTERN,
		/// @brief const
		TKN_KEYWORD_CONST,
		/// @brief In parameter
		TKN_KEYWORD_IN,
		/// @brief Out parameter
		TKN_KEYWORD_OUT,
		/// @brief Inout parameter
		TKN_KEYWORD_INOUT,
		/// @brief Move parameter
		TKN_KEYWORD_MOVE,
		/// @brief Copy parameter
		TKN_KEYWORD_COPY,
		
		/// @brief typeof
		TKN_KEYWORD_TYPEOF,

		/// @brief sizeof
		TKN_KEYWORD_SIZEOF,
		/// @brief alignof
		TKN_KEYWORD_ALIGNOF,
		/// @brief alignas
		TKN_KEYWORD_ALIGNAS,

		/// @brief as
		TKN_KEYWORD_AS,
		/// @brief bit_as
		TKN_KEYWORD_BIT_AS,
		/// @brief using
		TKN_KEYWORD_USING,
		/// @brief public
		TKN_KEYWORD_PUBLIC,
		/// @brief private
		TKN_KEYWORD_PRIVATE,
		/// @brief namespace
		TKN_KEYWORD_MODULE,

		/// @brief switch
		TKN_KEYWORD_SWITCH,
		/// @brief case
		TKN_KEYWORD_CASE,
		/// @brief default
		TKN_KEYWORD_DEFAULT,

		/// @brief goto
		TKN_KEYWORD_GOTO,

		/********* ADD NEW KEYWORDS BEGINNING HERE *******/


		/// @brief any identifier
		TKN_IDENTIFIER,
		/// @brief \.
		TKN_DOT,
		/// @brief //... or multi-line comments
		TKN_COMMENT
	};
}

namespace clt::lng
{
	/// @brief Check if a Token represents any assignment Token (=, +=, ...)
	/// @param tkn The token to check for
	/// @return True if the Token is an assignment Token
	constexpr bool isAssignmentToken(Token tkn) noexcept
	{
		return Token::TKN_EQUAL <= tkn
			&& tkn <= Token::TKN_GREAT_GREAT_EQUAL;
	}

	/// @brief Check if Token represents any direct assignment (+=, -=, ...)
	/// @param tkn The token to check for
	/// @return True if the Token is an direct assignment Token
	constexpr bool isDirectAssignmentToken(Token tkn) noexcept
	{
		return Token::TKN_EQUAL < tkn
			&& tkn <= Token::TKN_GREAT_GREAT_EQUAL;
	}

	/// @brief Converts a direct assignment to its non-assigning equivalent.
	/// Example: '+=' -> '+'
	/// @param tkn The direct assignment Token
	/// @return Non-assigning Token
	/// @pre isDirectAssignmentToken(tkn)
	constexpr Token DirectAssignToNonAssignToken(Token tkn) noexcept
	{
		assert_true("Invalid token!", isDirectAssignmentToken(tkn));
		return static_cast<Token>(static_cast<u8>(tkn) - 19);
	}

	/// @brief Check if a Token represents any comparison Token (==, !=, ...)
	/// '||' and '&&' are considered comparison tokens.
	/// @param tkn The token to check for
	/// @return True if the Token is a comparison Token
	constexpr bool isComparisonToken(Token tkn) noexcept
	{
		return Token::TKN_AND_AND < tkn
			&& tkn <= Token::TKN_EQUAL_EQUAL;
	}

	/// @brief Check if a Token represents any literal token ('.', "...", ...)
	/// @param tkn The token to check for
	/// @return True if the Token is a literal token
	constexpr bool isLiteralToken(Token tkn) noexcept
	{
		return Token::TKN_BOOL_L < tkn
			&& tkn < Token::TKN_STRING_L;
	}

	/// @brief Check if a Token represents any unary operator (&, ++, ...)
	/// @param tkn The token to check for
	/// @return True if the Token is a unary operator token
	constexpr bool isUnaryToken(Token tkn) noexcept
	{
		return (Token::TKN_PLUS <= tkn && tkn <= Token::TKN_STAR)
			|| tkn == Token::TKN_AND
			|| tkn == Token::TKN_PLUS_PLUS
			|| tkn == Token::TKN_MINUS_MINUS
			|| tkn == Token::TKN_TILDE
			|| tkn == Token::TKN_BANG;
	}

	/// @brief Check if a Token represents any binary operator (+, -, ...)
	/// @param tkn The token to check for
	/// @return True if the Token is a binary operator token
	constexpr bool isBinaryToken(Token tkn) noexcept
	{
		return tkn <= Token::TKN_GREAT_GREAT_EQUAL;
	}

	/// @brief Check if a Token is a built-in type keyword (TKN_KEYWORD_VOID...)
	/// @param tkn The token to check for
	/// @return True if any TKN_KEYWORD_* that represents a type (PTR is not a type)
	constexpr bool isBuiltinToken(Token tkn) noexcept
	{
		return Token::TKN_KEYWORD_VOID <= tkn && tkn <= Token::TKN_KEYWORD_QWORD;
	}
}

#endif //!HG_COLT_TOKEN