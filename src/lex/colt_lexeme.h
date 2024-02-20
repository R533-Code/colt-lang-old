/*****************************************************************//**
 * @file   colt_lexeme.h
 * @brief  Contains an enum for all valid tokens (representing a lexeme)
 *				 of the Colt language.
 * 
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_LEXEME
#define HG_COLT_LEXEME

#include "util/assertions.h"
#include "util/types.h"

namespace clt::lng
{
	/// @brief Represents the lexeme of the Colt language.
	enum class Lexeme: u8;	
}

DECLARE_ENUM_WITH_TYPE(u8, clt::lng, Lexeme,
	/********* BEGINNING OF BINARY OPERATORS *******/

	/// @brief +
	TKN_PLUS,
	/// @brief -
	TKN_MINUS,
	/// @brief *
	TKN_STAR,

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
	TKN_EXCLAM_EQUAL,
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
	TKN_EXCLAM,

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
		);

namespace clt::lng
{
	/// @brief Check if a Lexeme represents any assignment Lexeme (=, +=, ...)
	/// @param tkn The token to check for
	/// @return True if the Lexeme is an assignment Lexeme
	constexpr bool isAssignmentToken(Lexeme tkn) noexcept
	{
		return Lexeme::TKN_EQUAL <= tkn
			&& tkn <= Lexeme::TKN_GREAT_GREAT_EQUAL;
	}

	/// @brief Check if Lexeme represents any direct assignment (+=, -=, ...)
	/// @param tkn The token to check for
	/// @return True if the Lexeme is an direct assignment Lexeme
	constexpr bool isDirectAssignmentToken(Lexeme tkn) noexcept
	{
		return Lexeme::TKN_EQUAL < tkn
			&& tkn <= Lexeme::TKN_GREAT_GREAT_EQUAL;
	}

	/// @brief Converts a direct assignment to its non-assigning equivalent.
	/// Example: '+=' -> '+'
	/// @param tkn The direct assignment Lexeme
	/// @return Non-assigning Lexeme
	/// @pre isDirectAssignmentToken(tkn)
	constexpr Lexeme DirectAssignToNonAssignToken(Lexeme tkn) noexcept
	{
		assert_true("Invalid token!", isDirectAssignmentToken(tkn));
		return static_cast<Lexeme>(static_cast<u8>(tkn) - 19);
	}

	/// @brief Check if a Lexeme represents any comparison Lexeme (==, !=, ...)
	/// '||' and '&&' are considered comparison tokens.
	/// @param tkn The token to check for
	/// @return True if the Lexeme is a comparison Lexeme
	constexpr bool isComparisonToken(Lexeme tkn) noexcept
	{
		return Lexeme::TKN_AND_AND < tkn
			&& tkn <= Lexeme::TKN_EQUAL_EQUAL;
	}

	/// @brief Check if a Lexeme represents any literal token ('.', "...", ...)
	/// @param tkn The token to check for
	/// @return True if the Lexeme is a literal token
	constexpr bool isLiteralToken(Lexeme tkn) noexcept
	{
		return Lexeme::TKN_BOOL_L < tkn
			&& tkn < Lexeme::TKN_STRING_L;
	}

	/// @brief Check if a Lexeme represents any unary operator (&, ++, ...)
	/// @param tkn The token to check for
	/// @return True if the Lexeme is a unary operator token
	constexpr bool isUnaryToken(Lexeme tkn) noexcept
	{
		return (Lexeme::TKN_PLUS <= tkn && tkn <= Lexeme::TKN_STAR)
			|| tkn == Lexeme::TKN_AND
			|| tkn == Lexeme::TKN_PLUS_PLUS
			|| tkn == Lexeme::TKN_MINUS_MINUS
			|| tkn == Lexeme::TKN_TILDE
			|| tkn == Lexeme::TKN_EXCLAM;
	}

	/// @brief Check if a Lexeme represents any binary operator (+, -, ...)
	/// @param tkn The token to check for
	/// @return True if the Lexeme is a binary operator token
	constexpr bool isBinaryToken(Lexeme tkn) noexcept
	{
		return tkn <= Lexeme::TKN_GREAT_GREAT_EQUAL;
	}

	/// @brief Check if a Lexeme is a built-in type keyword (TKN_KEYWORD_VOID...)
	/// @param tkn The token to check for
	/// @return True if any TKN_KEYWORD_* that represents a type (PTR is not a type)
	constexpr bool isBuiltinToken(Lexeme tkn) noexcept
	{
		return Lexeme::TKN_KEYWORD_VOID <= tkn && tkn <= Lexeme::TKN_KEYWORD_QWORD;
	}
}

#endif //!HG_COLT_LEXEME