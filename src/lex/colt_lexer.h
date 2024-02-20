/*****************************************************************//**
 * @file   colt_lexer.h
 * @brief  Contains the lexing functions.
 * 
 * @author RPC
 * @date   February 2024
 *********************************************************************/
#ifndef HG_COLT_LEXER
#define HG_COLT_LEXER

#include <array>
#include <exception>
#include "ascii.h"
#include "err/error_reporter.h"
#include "colt_token_buffer.h"
#include "util/exit_recursion.h"

namespace clt::lng
{	
	/**
	* Functions starting with 'Consume' do not add any Token
	* to the token buffer.
	* Functions starting with 'Parse' add a Token to the
	* token buffer.
	*/

	/// @brief Consumes all characters till a whitespace is hit
	/// @param lexer The lexer used for parsing
	void ConsumeTillWhitespaces(Lexer& lexer) noexcept;

	/// @brief Consumes all whitespaces till a non-whitespace is hit
	/// @param lexer The lexer used for parsing
	void ConsumeWhitespaces(Lexer& lexer) noexcept;

	/// @brief Consumes all multi-line comments (recursive)
	/// @param lexer The lexer used for parsing
	/// @pre The '/*' of the comment must be consumed
	void ConsumeLinesComment(Lexer& lexer) noexcept;

	/// @brief Consumes all the digits (saving them in `lexer.temp`).
	/// This function does not clear `temp`.
	/// @param lexer The lexer used for parsing
	void ConsumeDigits(Lexer& lexer) noexcept;

	/// @brief Parses an invalid character (consuming till a whitespace is hit)
	/// @param lexer The lexer used for parsing
	void ParseInvalid(Lexer& lexer) noexcept;

	/// @brief Parses a '+'
	/// @param lexer The lexer used for parsing
	void ParsePlus(Lexer& lexer) noexcept;

	/// @brief Parses a '-'
	/// @param lexer The lexer used for parsing
	void ParseMinus(Lexer& lexer) noexcept;

	/// @brief Parses a '*'
	/// @param lexer The lexer used for parsing
	void ParseStar(Lexer& lexer) noexcept;

	/// @brief Parses a '/'
	/// @param lexer The lexer used for parsing
	void ParseSlash(Lexer& lexer) noexcept;

	/// @brief Parses a '%'
	/// @param lexer The lexer used for parsing
	void ParsePercent(Lexer& lexer) noexcept;

	/// @brief Parses a ':'
	/// @param lexer The lexer used for parsing
	void ParseColon(Lexer& lexer) noexcept;

	/// @brief Parses a '='
	/// @param lexer The lexer used for parsing
	void ParseEqual(Lexer& lexer) noexcept;

	/// @brief Parses a '!'
	/// @param lexer The lexer used for parsing
	void ParseExclam(Lexer& lexer) noexcept;

	/// @brief Parses a '<'
	/// @param lexer The lexer used for parsing
	void ParseLt(Lexer& lexer) noexcept;
	
	/// @brief Parses a '>'
	/// @param lexer The lexer used for parsing
	void ParseGt(Lexer& lexer) noexcept;
	
	/// @brief Parses a '&'
	/// @param lexer The lexer used for parsing
	void ParseAnd(Lexer& lexer) noexcept;

	/// @brief Parses a '|'
	/// @param lexer The lexer used for parsing
	void ParseOr(Lexer& lexer) noexcept;

	/// @brief Parses a '^'
	/// @param lexer The lexer used for parsing
	void ParseCaret(Lexer& lexer) noexcept;
	
	/// @brief Parses a '^'
	/// @param lexer The lexer used for parsing
	void ParseCaret(Lexer& lexer) noexcept;

	template<Lexeme lexeme>
	/// @brief Parses a single character, adding 'lexeme' to the token buffer.
	/// @tparam lexeme The lexeme to add to the token buffer
	/// @param lexer The lexer used for parsing
	void ParseSingle(Lexer& lexer) noexcept;

	/// @brief Parses a '\.'
	/// @param lexer The lexer used for parsing
	void ParseDot(Lexer& lexer) noexcept;

	template<typename T>
	/// @brief Check if a lexeme matches a type (used for assertions)
	/// @tparam T The type that the lexeme should represent
	/// @param lex The lexeme that should represent 'T'
	/// @return True if 'lex' represents 'T'
	constexpr bool CheckLiteralLexeme(Lexeme lex) noexcept;

	/// @brief The type of the lexing functions callbacks
	using LexerDispatch_t = void(*)(Lexer&) noexcept;

	/// @brief Table containing lexing functions used for dispatch
	struct LexerDispatchTable
		: public std::array<LexerDispatch_t, 256>
	{};

	consteval LexerDispatchTable GenLexerDispatchTable() noexcept
	{
		LexerDispatchTable table{};
		for (size_t i = 0; i < table.size(); i++)
		{
			if (clt::isspace((char)i))
				table[i] = &ConsumeWhitespaces;
			else
				table[i] = &ParseInvalid;
		}		

		table['+'] = &ParsePlus;
		table['-'] = &ParseMinus;
		table['*'] = &ParsePlus;
		table['/'] = &ParseSlash;
		table['%'] = &ParsePercent;
		table[':'] = &ParseColon;
		table['='] = &ParseEqual;
		table['!'] = &ParseExclam;
		table['.'] = &ParseDot;
		table['<'] = &ParseLt;
		table['>'] = &ParseGt;
		table['&'] = &ParseAnd;
		table['|'] = &ParseOr;
		table['^'] = &ParseCaret;
		table['~'] = &ParseSingle<Lexeme::TKN_TILDE>;
		table[';'] = &ParseSingle<Lexeme::TKN_SEMICOLON>;
		table[','] = &ParseSingle<Lexeme::TKN_COLON>;
		return table;
	}

	struct Lexer
	{
		/// @brief The error reporter
		ErrorReporter& reporter;
		/// @brief TokenBuffer where to save the lexemes
		TokenBuffer& buffer;
		/// @brief The current line
		u32 line_nb = 0;
		/// @brief The current offset into the current line
		u32 offset = 0;
		/// @brief Temporary string used for literals
		String temp = {};
		/// @brief The size of the current lexeme
		u32 size_lexeme = 0;
		/// @brief Recursion depth for parsing comments
		u8 comment_depth = 0;
		/// @brief Next character to parse
		char next;

		struct Snapshot
		{
			u32 line_nb;
			u32 column;
		};

		/// @brief Size of the beginning of a multiline comment (SLASH STAR)
		static constexpr u32 MultilineCommentSize = 2;
		/// @brief The lexing table used to dispatch
		static constexpr LexerDispatchTable LexingTable = GenLexerDispatchTable();

		constexpr char getNext() noexcept
		{
			if (line_nb == buffer.lines.size())
				return EOF;
			++size_lexeme;
			if (offset == buffer.lines[line_nb].size())
			{
				offset = 0;
				++line_nb;
				return getNext();
			}
			return buffer.lines[line_nb][offset++];
		}

		constexpr u32 getOffset() const noexcept
		{
			assert_true("getOffset can only be called after a call to getNext!",
				!(offset == 0 && line_nb == 0));
			
			return offset ? offset - 1
				: static_cast<u32>(buffer.lines[line_nb - 1].size()) - 1;
		}

		constexpr Snapshot startLexeme() noexcept
		{
			assert_true("startLexeme can only be called after a call to getNext!",
				!(offset == 0 && line_nb == 0));
			
			size_lexeme = 0;
			return Snapshot{ line_nb, offset - 1 };
		}

		/// @brief Look ahead in the string to scan
		/// @param offset The offset to add (0 being look ahead 1 character)
		/// @return The character 'offset + 1' after the current one
		constexpr char peekNext(u32 offset = 0) const noexcept
		{
			auto i = (i64)(this->offset + offset) - (i64)buffer.lines[line_nb].size();
			if (i < 0)
				return buffer.lines[line_nb][this->offset + offset];
			const auto next_line = line_nb + 1;
			if (next_line == buffer.lines.size())
				return EOF;
			return buffer.lines[next_line][i];
		}

		/// @brief Creates a SourceInfo over a single-line lexeme
		/// @param start The start offset of the lexeme
		/// @return SourceInfo over the lexeme
		constexpr SourceInfo makeSource(const Snapshot& snap) const noexcept
		{
			return SourceInfo{ snap.line_nb, StringView{ &buffer.lines[snap.line_nb].front() + snap.column, size_lexeme }, buffer.lines[snap.line_nb] };
		}

		/// @brief Creates a SourceInfo over a single-line lexeme
		/// @param start The start offset of the lexeme
		/// @return SourceInfo over the lexeme
		constexpr SourceInfo makeSource(u32 line_nb, u32 start) const noexcept
		{
			return SourceInfo{ line_nb, StringView{ &buffer.lines[line_nb].front() + start, size_lexeme }, buffer.lines[line_nb] };
		}

		/// @brief Creates a SourceInfo over a single-line lexeme
		/// @param start The start offset of the lexeme
		/// @param end The end offset of the lexeme
		/// @return SourceInfo over the lexeme
		constexpr SourceInfo makeSource(u32 line_nb, u32 start, u32 end) const noexcept
		{
			return SourceInfo{ line_nb, StringView{ &buffer.lines[line_nb].front() + start, end - start }, buffer.lines[line_nb] };
		}

		/// @brief Saves a Token in the TokenBuffer
		/// @param lexeme The lexeme of the Token
		/// @param column The starting column of the Token
		void addToken(Lexeme lexeme, const Snapshot& snap) const noexcept
		{
			assert_true("Invalid call to addToken", size_lexeme != 0);
			buffer.addToken(lexeme, snap.line_nb, snap.column, size_lexeme);
		}

		template<typename T>
		void addLiteral(Lexeme lexeme, T value, const Snapshot& snap) const noexcept
		{
			assert_true("Verify lexeme and literal type!", CheckLiteralLexeme<T>(lexeme));
			QWORD_t literal{};
			literal.bit_assign(value);
			buffer.addLiteral(literal, lexeme, snap.line_nb, snap.column, size_lexeme);
		}
	};	

	template<Lexeme lexeme>
	void ParseSingle(Lexer& lexer) noexcept
	{
		auto snap = lexer.startLexeme();
		lexer.next = lexer.getNext();
		lexer.addToken(lexeme, snap);
	}

	template<typename T>
	constexpr bool CheckLiteralLexeme(Lexeme lex) noexcept
	{
		switch (lex)
		{
			using enum Lexeme;
		case TKN_BOOL_L:
			return std::same_as<T, bool>;
		case TKN_CHAR_L:
			return std::same_as<T, char>;
		case TKN_U8_L:
			return std::same_as<T, u8>;
		case TKN_U16_L:
			return std::same_as<T, u16>;
		case TKN_U32_L:
			return std::same_as<T, u32>;
		case TKN_U64_L:
			return std::same_as<T, u64>;
		case TKN_I8_L:
			return std::same_as<T, i8>;
		case TKN_I16_L:
			return std::same_as<T, i16>;
		case TKN_I32_L:
			return std::same_as<T, i32>;
		case TKN_I64_L:
			return std::same_as<T, i64>;
		case TKN_FLOAT_L:
			return std::same_as<T, f32>;
		case TKN_DOUBLE_L:
			return std::same_as<T, f64>;
		default:
			return false;
		}
	}
}

#endif // !HG_COLT_LEXER
