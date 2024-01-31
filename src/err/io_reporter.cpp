/*****************************************************************//**
 * @file   io_reporter.cpp
 * @brief  Definitions of generate_* functions.
 * 
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#include "io_reporter.h"

namespace clt::lng
{
	/// @brief Prints a single line
	/// @param highlight The color to use when highlighting
	/// @param src_info The information to highlight
	/// @param begin_line The beginning of the line
	/// @param end_line The end of the line
	/// @param line_nb_size The size of the line number static_cast a string
	void print_single_line(io::Color highlight, const SourceInfo& src_info, StringView begin_line, StringView end_line, size_t line_nb_size) noexcept
	{
		//TODO: implement HighlightCode
		io::print(" {} | {}{}{}{}{}", src_info.line_begin, /*io::HighlightCode*/ begin_line,
			highlight, src_info.expr, io::Reset, /*io::HighlightCode*/ end_line);

		auto sz = src_info.expr.size();
		//So no overflow happens when the expr is empty
		sz += static_cast<size_t>(sz == 0);
		sz -= 1;
		io::print(" {: <{}} | {: <{}}{:~<{}}^", "", line_nb_size, "", begin_line.size(), "", sz);
	}

	/// @brief Prints multiple lines
	/// @param highlight The color to use when highlighting
	/// @param src_info The information to highlight
	/// @param begin_line The beginning of the line
	/// @param end_line The end of the line
	/// @param line_nb_size The size of the line number static_cast a string
	void print_multiple_lines(io::Color highlight, const SourceInfo& src_info, StringView begin_line, StringView end_line, size_t line_nb_size) noexcept
	{
		size_t offset = StringView::npos; //will overflow on first add
		size_t previous_offset = 0;
		size_t current_line = src_info.line_begin;
		for (;;)
		{
			// +1 to skip over '\n'
			//As offset start with npos, offset + 1 == 0 on first iteration
			previous_offset = offset + 1;
			offset = begin_line.find('\n', offset + 1);

			if (offset == StringView::npos)
			{
				offset = src_info.expr.find('\n', 0);
				offset *= static_cast<size_t>(offset != StringView::npos);
				break;
			}

			io::print(" {: >{}} | {}", current_line, line_nb_size,
				/*io::HighlightCode*/StringView{ begin_line.data() + previous_offset, begin_line.data() + offset });
			++current_line;
		}
		io::print(" {: >{}} | {}{}{}{}", current_line, line_nb_size,
			/*io::HighlightCode*/StringView{ begin_line.data() + previous_offset, begin_line.data() + begin_line.size() }, highlight,
			StringView{ src_info.expr.data(), src_info.expr.data() + offset }, io::Reset);
		++current_line;
		for (;;)
		{
			previous_offset = offset + 1;
			offset = src_info.expr.find('\n', offset + 1);

			if (offset == StringView::npos)
			{
				offset = end_line.find('\n', 0);
				offset *= static_cast<size_t>(offset != StringView::npos);
				offset += end_line.size() * static_cast<size_t>(offset != StringView::npos);
				break;
			}

			io::print(" {: >{}} | {}{}{}", current_line, line_nb_size, highlight,
				StringView{ src_info.expr.data() + previous_offset, src_info.expr.data() + offset }, io::Reset);
			++current_line;
		}
		io::print(" {: >{}} | {}{}{}{}", current_line, line_nb_size, highlight,
			StringView{ src_info.expr.data() + previous_offset, src_info.expr.data() + src_info.expr.size()}, io::Reset,
			/*io::HighlightCode*/ StringView{ end_line.data(), end_line.data() + offset });
		++current_line;
		for (;;)
		{
			previous_offset = offset + 1;
			offset = end_line.find('\n', offset + 1);

			if (offset == StringView::npos)
			{
				if (previous_offset < end_line.size())
				{
					io::print(" {: >{}} | {}", current_line, line_nb_size,
						/*io::HighlightCode*/StringView{ end_line.data() + previous_offset, end_line.data() + end_line.size() });
				}
				break;
			}

			io::print(" {: >{}} | {}", current_line, line_nb_size,
				/*io::HighlightCode*/ StringView{ end_line.data() + previous_offset, end_line.data() + offset });
			++current_line;
		}
	}

	void handle_valid_src(const SourceInfo& src_info, io::Color color) noexcept
	{
		StringView begin_line = { src_info.lines.data(), src_info.expr.data() };
		StringView end_line = { src_info.lines.data() + src_info.expr.size(), src_info.lines.data() + src_info.lines.size() };

		//If lexeme.size() == 0, then the lexeme will be outside of the line_strv:
		//This is because the only case where the lexeme is empty is due to reaching
		//the last lexeme.
		if (src_info.expr.size() == 0)
		{
			end_line = StringView{ src_info.expr.data(), src_info.expr.data() };
			begin_line = src_info.lines;
		}
		else
			end_line = StringView{ src_info.expr.data() + src_info.expr.size(), src_info.lines.data() + src_info.lines.size() };

		size_t line_nb_size = fmt::formatted_size("{}", src_info.line_end);
		if (src_info.is_single_line())
			print_single_line(io::CyanF, src_info, begin_line, end_line, line_nb_size);
		else
			print_multiple_lines(io::CyanF, src_info, begin_line, end_line, line_nb_size);
	}

  void generate_message(StringView fmt, const Option<SourceInfo>& src, const Option<ReportNumber>& nb) noexcept
  {
		if (nb.is_value())
			io::print_message("{}", fmt);
		else
			io::print_message("(M{}) {}", nb.value(), fmt);

		if (src.is_value())
			handle_valid_src(src.value(), io::CyanF);
  }

	void generate_warn(StringView fmt, const Option<SourceInfo>& src, const Option<ReportNumber>& nb) noexcept
	{
		if (nb.is_value())
			io::print_warn("{}", fmt);
		else
			io::print_warn("(W{}) {}", nb.value(), fmt);

		if (src.is_value())
			handle_valid_src(src.value(), io::YellowF);
	}

	void generate_error(StringView fmt, const Option<SourceInfo>& src, const Option<ReportNumber>& nb) noexcept
	{
		if (nb.is_value())
			io::print_error("{}", fmt);
		else
			io::print_error("(E{}) {}", nb.value(), fmt);

		if (src.is_value())
			handle_valid_src(src.value(), io::BrightRedB);
	}
}
