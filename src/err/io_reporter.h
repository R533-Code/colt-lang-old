/*****************************************************************//**
 * @file   io_reporter.h
 * @brief  Helpers for error reporting from the compiler.
 * 
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_IO_REPORTER
#define HG_COLT_IO_REPORTER

#include "util/types.h"
#include "structs/string.h"

namespace clt::lng
{
	/// @brief ID of error/warning/message
	using ReportNumber = u32;

	/// @brief The source code information of an expression.
	struct SourceInfo
	{
		/// @brief The beginning line number of the expression (1-based)
		u32 line_begin;
		/// @brief The end line number of the expression (1-based)
		u32 line_end;
		/// @brief StringView over all the lines on which the expression spans
		StringView lines;
		/// @brief StringView over the expression (included in lines)
		StringView expr;

		SourceInfo() = delete;
		constexpr SourceInfo(const SourceInfo&)             noexcept = default;
		constexpr SourceInfo(SourceInfo&&)                  noexcept = default;
		constexpr SourceInfo& operator=(SourceInfo&&)       noexcept = default;
		constexpr SourceInfo& operator=(const SourceInfo&)  noexcept = default;
		constexpr SourceInfo(u32 line, StringView expr, StringView line_str) noexcept
			: line_begin(line), line_end(line), lines(line_str), expr(expr) {}
		constexpr SourceInfo(u32 line_s, u32 line_e, StringView expr, StringView line_str) noexcept
			: line_begin(line_s), line_end(line_e), lines(line_str), expr(expr) {}

		/// @brief Concatenates two SourceInfo
		/// @param rhs The right hand side
		constexpr void concat(const SourceInfo& rhs) noexcept
		{
			line_end = rhs.line_end;
			lines = StringView{ lines.begin(), rhs.lines.end() };
			expr = StringView{ expr.begin(), rhs.expr.end() };
		}

		/// @brief Check if the information represents a single line
		/// @return True if line_begin == line_end
		constexpr bool is_single_line() const noexcept { return line_begin == line_end; }
	};

	/// @brief Function pointer type of generate_* functions
	using report_print_t = void(*)(StringView, const Option<SourceInfo>&, const Option<ReportNumber>&) noexcept;

	/// @brief Prints a message to the console, highlighting code
	/// @param str The message
	/// @param src_info The message information (or None)
	/// @param nb The message number (or None)
	void generate_message(StringView str, const Option<SourceInfo>& src_info, const Option<ReportNumber>& nb) noexcept;
	/// @brief Prints a warning to the console, highlighting code
	/// @param str The warning
	/// @param src_info The warning information (or None)
	/// @param nb The warning number (or None)
	void		generate_warn(StringView str, const Option<SourceInfo>& src_info, const Option<ReportNumber>& nb) noexcept;
	/// @brief Prints an error to the console, highlighting code
	/// @param str The error
	/// @param src_info The error information (or None)
	/// @param nb The error number (or None)
	void   generate_error(StringView str, const Option<SourceInfo>& src_info, const Option<ReportNumber>& nb) noexcept;
}

#endif // !HG_COLT_IO_REPORTER
