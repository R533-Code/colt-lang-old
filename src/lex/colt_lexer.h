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
	struct Lexer;

	/**
	* Functions starting with 'Consume' do not add any Token
	* to the token buffer.
	* Functions starting with 'Parse' add a Token to the
	* token buffer.
	*/

	/// @brief Consumes all characters till a whitespace is hit
	/// @param lexer The lexer used for parsing
	void ConsumeTillWhitespaces(Lexer& lexer) noexcept;
	
	/// @brief Consumes all characters till a whitespace or a punctuation is hit
	/// @param lexer The lexer used for parsing
	void ConsumeTillSpaceOrPunct(Lexer& lexer) noexcept;

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

	/// @brief Consumes all the digits (with base 'base'), saving them in `lexer.temp`.
	/// This function does not clear `temp`.
	/// @param lexer The lexer used for parsing
	void ConsumeDigits(Lexer& lexer, int base) noexcept;

	/// @brief Consumes all the alpha-numeric characters (saving them in `lexer.temp`).
	/// This function does not clear `temp`.
	/// @param lexer The lexer used for parsing
	void ConsumeAlnum(Lexer& lexer) noexcept;

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

	/// @brief Parses digits (floats or integrals)
	/// @param lexer The lexer used for parsing
	void ParseDigit(Lexer& lexer) noexcept;

	/// @brief Parses an identifier
	/// @param lexer The lexer used for parsing
	void ParseIdentifier(Lexer& lexer) noexcept;

	template<Lexeme lexeme>
	/// @brief Parses a single character, adding 'lexeme' to the token buffer.
	/// @tparam lexeme The lexeme to add to the token buffer
	/// @param lexer The lexer used for parsing
	void ParseSingle(Lexer& lexer) noexcept;

	/// @brief Parses a '\.'
	/// @param lexer The lexer used for parsing
	void ParseDot(Lexer& lexer) noexcept;

	template<typename T>
	/// @brief Returns the literal lexeme representing the C++ type 'T'
	/// @tparam T The type to convert to its lexeme equivalent
	/// @return The lexeme corresponding to 'T'
	constexpr Lexeme LiteralFromType() noexcept;

	/// @brief The type of the lexing functions callbacks
	using LexerDispatch_t = void(*)(Lexer&) noexcept;

	/// @brief Table containing lexing functions used for dispatch
	struct LexerDispatchTable
		: public std::array<LexerDispatch_t, 256>
	{};

	consteval LexerDispatchTable GenLexerDispatchTable() noexcept
	{
		LexerDispatchTable table{};
		for (char i = 0; i < table.size(); i++)
		{
			if (clt::isspace(i))
				table[i] = &ConsumeWhitespaces;
			else if (clt::isdigit(i))
				table[i] = &ParseDigit;
			else if (clt::isalpha(i))
				table[i] = &ParseIdentifier;
			else
				table[i] = &ParseInvalid;
		}

		table['_'] = &ParseIdentifier;
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

		template<typename... Args>
		StringView fmt(clt::io::fmt_str<Args...> fmt, Args&&... args) noexcept
		{
			return buffer.fmt(fmt, std::forward<Args>(args)...);
		}

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
			assert_true("Verify lexeme and literal type!", LiteralFromType<T>() == lexeme);
			QWORD_t literal{};
			literal.bit_assign(value);
			buffer.addLiteral(literal, lexeme, snap.line_nb, snap.column, size_lexeme);
		}
	};

	void HandleFloatWithExtension(Lexer& lexer, const Lexer::Snapshot& snap) noexcept;
	
	template<bool UnsignedOnly = false>
	void HandleIntWithExtension(Lexer& lexer, const Lexer::Snapshot& snap, int base = 10) noexcept;

	template<meta::FloatingPoint T, Lexeme lex>
	void parse_floating(Lexer& lexer, const Lexer::Snapshot& snap) noexcept;
	
	template<meta::Integral T, Lexeme lex>
	void parse_integral(Lexer& lexer, const Lexer::Snapshot& snap, int base = 10) noexcept;

	/**
	* Template function implementations
	*/

	template<Lexeme lexeme>
	void ParseSingle(Lexer& lexer) noexcept
	{
		auto snap = lexer.startLexeme();
		lexer.next = lexer.getNext();
		lexer.addToken(lexeme, snap);
	}	

	template<meta::FloatingPoint T, Lexeme lex>
	void parse_floating(Lexer& lexer, const Lexer::Snapshot& snap) noexcept
	{
		T value;
		auto result = clt::parse(lexer.temp, value);
		if (result.code() == ParsingCode::GOOD)
			return lexer.addLiteral(lex, value, snap);

		lexer.addToken(Lexeme::TKN_ERROR, snap);
		lexer.reporter.error(
			lexer.fmt("Invalid '{}' literal!", clt::reflect<T>::str()),
			lexer.makeSource(snap)
		);
	}

	template<meta::Integral T, Lexeme lex>
	void parse_integral(Lexer& lexer, const Lexer::Snapshot& snap, int base) noexcept
	{
		T value = 0;
		auto [ptr, err] = std::from_chars(lexer.temp.begin(), lexer.temp.end(), value, base);
		if (ptr == lexer.temp.end() && err == std::errc{})
			return lexer.addLiteral(lex, value, snap);

		lexer.addToken(Lexeme::TKN_ERROR, snap);
		lexer.reporter.error(
			lexer.fmt("Invalid '{}' literal!", clt::reflect<T>::str()),
			lexer.makeSource(snap)
		);
	}

	namespace details
	{
		template<typename dtype, typename found>
		void handle_1nb_int_extension(Lexer& lexer, const Lexer::Snapshot& snap, int base) noexcept
		{
			if (clt::isdigit(lexer.peekNext(1)))
				return parse_integral<dtype, LiteralFromType<dtype>()>(lexer, snap, base);
			// consume [iu]8
			lexer.getNext();
			lexer.next = lexer.getNext();
			return parse_integral<found, LiteralFromType<found>()>(lexer, snap, base);
		}

		template<typename dtype, typename found>
		void handle_2nb_int_extension(char chr, Lexer& lexer, const Lexer::Snapshot& snap, int base) noexcept
		{
			if (lexer.peekNext(1) != chr || clt::isdigit(lexer.peekNext(2)))
				return parse_integral<dtype, LiteralFromType<dtype>()>(lexer, snap, base);
			// consume [iu](16|32|64)
			lexer.getNext();
			lexer.getNext();
			lexer.next = lexer.getNext();
			return parse_integral<found, LiteralFromType<found>()>(lexer, snap, base);
		}
	}


	template<bool UnsignedOnly>
	void HandleIntWithExtension(Lexer& lexer, const Lexer::Snapshot& snap, int base) noexcept
	{
		// The default type to parse if no literal extension was found
		using dtype = std::conditional_t<UnsignedOnly, u64, i64>;

		switch (clt::tolower(lexer.next))
		{
		break; case 'u':
			switch (lexer.peekNext())
			{
			case '8':
				return details::handle_1nb_int_extension<dtype, u8>(lexer, snap, base);
			case '1':
				return details::handle_2nb_int_extension<dtype, u16>('6', lexer, snap, base);
			case '3':
				return details::handle_2nb_int_extension<dtype, u32>('2', lexer, snap, base);
			case '6':
				return details::handle_2nb_int_extension<dtype, u64>('4', lexer, snap, base);
			}
			// We make use of fallthrough if unsigned only
		break; case 'i': if constexpr (!UnsignedOnly) {
			switch (lexer.peekNext())
			{
			case '8':
				return details::handle_1nb_int_extension<dtype, i8>(lexer, snap, base);
			case '1':
				return details::handle_2nb_int_extension<dtype, i16>('6', lexer, snap, base);
			case '3':
				return details::handle_2nb_int_extension<dtype, i32>('2', lexer, snap, base);
			case '6':
				return details::handle_2nb_int_extension<dtype, i64>('4', lexer, snap, base);
			}
			break;
		}
		default:
			return parse_integral<dtype, LiteralFromType<dtype>()>(lexer, snap, base);
		}
	}

	template<typename T>
	constexpr Lexeme LiteralFromType() noexcept
	{
		using enum Lexeme;
		
		if constexpr (std::same_as<T, bool>)
			return TKN_BOOL_L;
		if constexpr (std::same_as<T, char>)
			return TKN_CHAR_L;
		if constexpr (std::same_as<T, u8>)
			return TKN_U8_L;
		if constexpr (std::same_as<T, u16>)
			return TKN_U16_L;
		if constexpr (std::same_as<T, u32>)
			return TKN_U32_L;
		if constexpr (std::same_as<T, u64>)
			return TKN_U64_L;
		if constexpr (std::same_as<T, i8>)
			return TKN_I8_L;
		if constexpr (std::same_as<T, i16>)
			return TKN_I16_L;
		if constexpr (std::same_as<T, i32>)
			return TKN_I32_L;
		if constexpr (std::same_as<T, i64>)
			return TKN_I64_L;
		if constexpr (std::same_as<T, f32>)
			return TKN_FLOAT_L;
		if constexpr (std::same_as<T, f64>)
			return TKN_DOUBLE_L;
	}
}

#endif // !HG_COLT_LEXER
