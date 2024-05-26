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
#include "lng/colt_builtin_id.h"

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
	// DO NOT ADD ANY KEYWORD BEFORE TKN_KEYWORD_IF
	// ALL KEYWORDS MUST START WITH TKN_KEYWORD_ FOLLOWED BY
	// THE ACTUAL KEYWORD

	/// @brief if
	TKN_KEYWORD_if,
	/// @brief elif
	TKN_KEYWORD_elif,
	/// @brief else
	TKN_KEYWORD_else,

	/// @brief for
	TKN_KEYWORD_for,
	/// @brief while
	TKN_KEYWORD_while,
	/// @brief break
	TKN_KEYWORD_break,
	/// @brief continue
	TKN_KEYWORD_continue,

	/********* VARIABLE DECLARATIONS *******/

	/// @brief var
	TKN_KEYWORD_var,
	/// @brief var
	TKN_KEYWORD_let,
	/// @brief mut
	TKN_KEYWORD_mut,
	/// @brief global
	TKN_KEYWORD_global,

	/********* BEGINNING OF BUILTIN TYPES *******/

		// DO NOT REORDER THESE TYPES

	/// @brief void
	TKN_KEYWORD_void,
	/// @brief bool
	TKN_KEYWORD_bool,
	/// @brief char
	TKN_KEYWORD_char,
	/// @brief u8
	TKN_KEYWORD_u8,
	/// @brief u16
	TKN_KEYWORD_u16,
	/// @brief u32
	TKN_KEYWORD_u32,
	/// @brief u64
	TKN_KEYWORD_u64,
	/// @brief i8
	TKN_KEYWORD_i8,
	/// @brief i16
	TKN_KEYWORD_i16,
	/// @brief i32
	TKN_KEYWORD_i32,
	/// @brief i64
	TKN_KEYWORD_i64,
	/// @brief f32
	TKN_KEYWORD_f32,
	/// @brief f64
	TKN_KEYWORD_f64,
	/// @brief BYTE
	TKN_KEYWORD_BYTE,
	/// @brief WORD
	TKN_KEYWORD_WORD,
	/// @brief DWORD
	TKN_KEYWORD_DWORD,
	/// @brief QWORD
	TKN_KEYWORD_QWORD,
	/// @brief ptr
	TKN_KEYWORD_ptr,
	/// @brief mutptr
	TKN_KEYWORD_mutptr,
	/// @brief opaque
	TKN_KEYWORD_opaque,
	/// @brief mutopaque
	TKN_KEYWORD_mutopaque,

	/********* END OF BUILTIN TYPES *******/

	/// @brief fn
	TKN_KEYWORD_fn,
	/// @brief return
	TKN_KEYWORD_return,
	/// @brief extern
	TKN_KEYWORD_extern,
	/// @brief const
	TKN_KEYWORD_const,
	/// @brief In parameter
	TKN_KEYWORD_in,
	/// @brief Out parameter
	TKN_KEYWORD_out,
	/// @brief Inout parameter
	TKN_KEYWORD_inout,
	/// @brief Move parameter
	TKN_KEYWORD_move,
	/// @brief Copy parameter
	TKN_KEYWORD_copy,

	/// @brief typeof
	TKN_KEYWORD_typeof,

	/// @brief sizeof
	TKN_KEYWORD_sizeof,
	/// @brief alignof
	TKN_KEYWORD_alignof,
	/// @brief alignas
	TKN_KEYWORD_alignas,

	/// @brief as
	TKN_KEYWORD_as,
	/// @brief bit_as
	TKN_KEYWORD_bit_as,
	/// @brief using
	TKN_KEYWORD_using,
	/// @brief public
	TKN_KEYWORD_public,
	/// @brief private
	TKN_KEYWORD_private,
	/// @brief module
	TKN_KEYWORD_module,

	/// @brief switch
	TKN_KEYWORD_switch,
	/// @brief case
	TKN_KEYWORD_case,
	/// @brief default
	TKN_KEYWORD_default,

	/// @brief goto
	TKN_KEYWORD_goto,
	/// @brief undefined
	TKN_KEYWORD_undefined,

	/********* ADD NEW KEYWORDS BEGINNING HERE *******/
	// Do not forget to add them to the table of keywords in getKeywordTable


	/// @brief any identifier
	TKN_IDENTIFIER,
	/// @brief \.
	TKN_DOT,
	/// @brief //... or multi-line comments
	TKN_COMMENT
		);

namespace clt::lng
{	
	/// @brief Transforms a literal token to a built-in ID
	/// @param tkn The token
	/// @return BuiltinID equivalent of the literal token
	constexpr BuiltinID LiteralToBuiltinID(Lexeme tkn) noexcept
	{
		assert_true("Token must be TKN_.*_L", Lexeme::TKN_BOOL_L <= tkn, tkn <= Lexeme::TKN_DOUBLE_L);
		return static_cast<BuiltinID>((u8)(tkn) - (u8)(Lexeme::TKN_BOOL_L));
	}

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
	/// '||' and '&&' are not considered comparison tokens.
	/// @param tkn The token to check for
	/// @return True if the Lexeme is a comparison Lexeme
	constexpr bool isComparisonToken(Lexeme tkn) noexcept
	{
		return Lexeme::TKN_LESS <= tkn
			&& tkn <= Lexeme::TKN_EQUAL_EQUAL;
	}
	
	/// @brief Check if a Lexeme represents any comparison Lexeme (==, !=, ...)
	/// '||' and '&&' are considered comparison tokens.
	/// @param tkn The token to check for
	/// @return True if the Lexeme is a comparison Lexeme
	constexpr bool isBoolProducerToken(Lexeme tkn) noexcept
	{
		return Lexeme::TKN_AND_AND <= tkn
			&& tkn <= Lexeme::TKN_EQUAL_EQUAL;
	}

	/// @brief Check if a Lexeme represents any literal token ('.', "...", ...)
	/// @param tkn The token to check for
	/// @return True if the Lexeme is a literal token
	constexpr bool isLiteralToken(Lexeme tkn) noexcept
	{
		return Lexeme::TKN_BOOL_L <= tkn
			&& tkn <= Lexeme::TKN_STRING_L;
	}

	/// @brief Check if a Lexeme represents any unary operator (&, ++, ...)
	/// @param tkn The token to check for
	/// @return True if the Lexeme is a unary operator token
	constexpr bool isUnaryToken(Lexeme tkn) noexcept
	{
		using enum clt::lng::Lexeme;
		return (TKN_PLUS <= tkn && tkn <= TKN_STAR)
			|| tkn == TKN_AND
			|| tkn == TKN_PLUS_PLUS
			|| tkn == TKN_MINUS_MINUS
			|| tkn == TKN_TILDE
			|| tkn == TKN_EXCLAM;
	}

	/// @brief Check if a Lexeme represents any binary operator (+, -, ...).
	/// This does not check for assignment operators.
	/// @param tkn The token to check for
	/// @return True if the Lexeme is a binary operator token
	constexpr bool isBinaryToken(Lexeme tkn) noexcept
	{
		return tkn <= Lexeme::TKN_EQUAL_EQUAL;
	}

	/// @brief Check if a Lexeme is a built-in type keyword (TKN_KEYWORD_bool...)
	/// @param tkn The token to check for
	/// @return True if any TKN_KEYWORD_* that represents a type (PTR and VOID are not types)
	constexpr bool isBuiltinToken(Lexeme tkn) noexcept
	{
		return Lexeme::TKN_KEYWORD_bool <= tkn && tkn <= Lexeme::TKN_KEYWORD_QWORD;
	}

	/// @brief Returns the keywords count (in Lexeme).
	/// This function counts the number of lexeme declared
	/// as TKN_KEYWORD_* in code.
	/// @return The number of keywords in Lexeme
	consteval u8 getKeywordCount() noexcept
	{
		u8 count = 0;
		for (auto i : reflect<Lexeme>::iter())
			if (reflect<Lexeme>::to_str(i).starts_with("TKN_KEYWORD_"))
				count++;
		return count;
	}

	/// @brief Returns a sorted map of the keywords string to
	/// their corresponding lexeme.
	/// @return Table from keyword string to lexeme
	consteval auto getKeywordMap() noexcept
	{
		constexpr u8 count = getKeywordCount();
		// Offset to the first keyword
		constexpr u8 first_key_offset = static_cast<u8>(Lexeme::TKN_KEYWORD_if);
		std::array<std::pair<std::string_view, Lexeme>, getKeywordCount()> array{};
		for (size_t i = 0; i < count; i++)
		{
			Lexeme lex = static_cast<Lexeme>(first_key_offset + i);
			std::string_view str = reflect<Lexeme>::to_str(lex);
			assert_true("Invalid Lexeme: all keywords must be contiguous!", str.find("TKN_KEYWORD_") != str.npos);
			// 12 for TKN_KEYWORD_
			str.remove_prefix(12);
			assert_true("Keyword size must at least be greater than 1!", str.size() > 1);
			array[i] = std::pair{ str, lex };
		}
		return meta::ConstexprMap{ array };
	}
}

#endif //!HG_COLT_LEXEME