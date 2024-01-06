/** @file print.h
* Contains print() functions to output to the console.
*/

#ifndef HG_COLT_PRINT
#define HG_COLT_PRINT

#include <utility>
#include <cstdio>

#include "io/color.h"
#include "meta/meta_string_literal.h"

namespace clt::io
{
  template<typename... Args>
  /// @brief Shorthand for fmt::format_string
  /// @tparam ...Args The types to be formatted to the string
  using fmt_str = fmt::format_string<Args...>;

  template<meta::StringLiteral endl = "\n", typename... Args>
  /// @brief Formats and prints a string to 'file'
  /// @tparam ...Args The types of the arguments to format
  /// @param file The file where to write the output
  /// @param fmt The format string
  /// @param ...args The arguments to format
  constexpr void print(std::FILE* file, fmt_str<Args...> fmt, Args&&... args) noexcept
  {
    if constexpr (endl.size() != 0)
    {
      fmt::basic_memory_buffer<char, 4096> buffer;
      fmt::format_to(std::back_inserter(buffer), fmt, std::forward<Args>(args)...);
      fmt::format_to(std::back_inserter(buffer), "{}", fmt::string_view{ endl.value, endl.size() });
      fmt::print(file, "{}", fmt::string_view{ buffer.data(), buffer.size() });
    }
    else
      fmt::print(file, fmt, std::forward<Args>(args)...);
  }

  template<meta::StringLiteral endl = "\n", typename... Args>
  /// @brief Formats and prints a string to 'file', prepending "Error: "
  /// @tparam ...Args The types of the arguments to format
  /// @param file The file where to write the output
  /// @param fmt The format string
  /// @param ...args The arguments to format
  constexpr void print_error(std::FILE* file, fmt_str<Args...> fmt, Args&&... args) noexcept
  {
    fmt::basic_memory_buffer<char, 4096> buffer;
    fmt::format_to(std::back_inserter(buffer), "{}Error:{} ", BrightRedF, Reset);
    fmt::format_to(std::back_inserter(buffer), fmt, std::forward<Args>(args)...);
    if constexpr (endl.size() != 0)
      fmt::format_to(std::back_inserter(buffer), "{}", fmt::string_view{ endl.value, endl.size() });
    fmt::print(file, "{}", fmt::string_view{ buffer.data(), buffer.size() });
  }

  template<meta::StringLiteral endl = "\n", typename... Args>
  /// @brief Formats and prints a string to 'file', prepending "Warning: "
  /// @tparam ...Args The types of the arguments to format
  /// @param file The file where to write the output
  /// @param fmt The format string
  /// @param ...args The arguments to format
  constexpr void print_warn(std::FILE* file, fmt_str<Args...> fmt, Args&&... args) noexcept
  {
    fmt::basic_memory_buffer<char, 4096> buffer;
    fmt::format_to(std::back_inserter(buffer), "{}Warning:{} ", BrightYellowF, Reset);
    fmt::format_to(std::back_inserter(buffer), fmt, std::forward<Args>(args)...);
    if constexpr (endl.size() != 0)
      fmt::format_to(std::back_inserter(buffer), "{}", fmt::string_view{ endl.value, endl.size() });
    fmt::print(file, "{}", fmt::string_view{ buffer.data(), buffer.size() });
  }

  template<meta::StringLiteral endl = "\n", typename... Args>
  /// @brief Formats and prints a string to 'file', prepending "Message: "
  /// @tparam ...Args The types of the arguments to format
  /// @param file The file where to write the output
  /// @param fmt The format string
  /// @param ...args The arguments to format
  constexpr void print_message(std::FILE* file, fmt_str<Args...> fmt, Args&&... args) noexcept
  {
    fmt::basic_memory_buffer<char, 4096> buffer;
    fmt::format_to(std::back_inserter(buffer), "{}Message:{} ", BrightBlueF, Reset);
    fmt::format_to(std::back_inserter(buffer), fmt, std::forward<Args>(args)...);
    if constexpr (endl.size() != 0)
      fmt::format_to(std::back_inserter(buffer), "{}", fmt::string_view{ endl.value, endl.size() });
    fmt::print(file, "{}", fmt::string_view{ buffer.data(), buffer.size() });
  }

  template<meta::StringLiteral endl = "\n", typename... Args>
  /// @brief Formats and prints a string to 'file', prepending "FATAL: "
  /// @tparam ...Args The types of the arguments to format
  /// @param file The file where to write the output
  /// @param fmt The format string
  /// @param ...args The arguments to format
  constexpr void print_fatal(std::FILE* file, fmt::format_string<Args...> fmt, Args && ...args)
  {
    fmt::basic_memory_buffer<char, 4096> buffer;
    fmt::format_to(std::back_inserter(buffer), "{}FATAL:{} {}", RedB, Reset, BrightRedF);
    fmt::format_to(std::back_inserter(buffer), fmt, std::forward<Args>(args)...);
    if constexpr (endl.size() != 0)
      fmt::format_to(std::back_inserter(buffer), "{}", fmt::string_view{ endl.value, endl.size() });
    fmt::print(file, "{}{}", fmt::string_view{ buffer.data(), buffer.size() }, io::Reset);
  }

  template<meta::StringLiteral endl = "\n", typename... Args>
  /// @brief Formats and prints a string to 'stdout'
  /// @tparam ...Args The types of the arguments to format
  /// @param fmt The format string
  /// @param ...args The arguments to format
  constexpr void print(fmt_str<Args...> fmt, Args&&... args) noexcept
  {
    print<endl>(stdout, fmt, std::forward<Args>(args)...);
  }

  template<meta::StringLiteral endl = "\n", typename... Args>
  /// @brief Formats and prints a string to 'stdout', prepending "Error: "
  /// @tparam ...Args The types of the arguments to format
  /// @param fmt The format string
  /// @param ...args The arguments to format
  constexpr void print_error(fmt_str<Args...> fmt, Args&&... args) noexcept
  {
    print_error<endl>(stdout, fmt, std::forward<Args>(args)...);
  }

  template<meta::StringLiteral endl = "\n", typename... Args>
  /// @brief Formats and prints a string to 'stdout', prepending "Warning: "
  /// @tparam ...Args The types of the arguments to format
  /// @param fmt The format string
  /// @param ...args The arguments to format
  constexpr void print_warn(fmt_str<Args...> fmt, Args&&... args) noexcept
  {
    print_warn<endl>(stdout, fmt, std::forward<Args>(args)...);
  }

  template<meta::StringLiteral endl = "\n", typename... Args>
  /// @brief Formats and prints a string to 'stdout', prepending "Message: "
  /// @tparam ...Args The types of the arguments to format
  /// @param fmt The format string
  /// @param ...args The arguments to format
  constexpr void print_message(fmt_str<Args...> fmt, Args&&... args) noexcept
  {
    print_message<endl>(stdout, fmt, std::forward<Args>(args)...);
  }

  template<meta::StringLiteral endl = "\n", typename... Args>
  /// @brief Formats and prints a string to 'stdout', prepending "FATAL: "
  /// @tparam ...Args The types of the arguments to format
  /// @param fmt The format string
  /// @param ...args The arguments to format
  constexpr void print_fatal(fmt::format_string<Args...> fmt, Args&&... args)
  {
    print_fatal<endl>(stdout, fmt, std::forward<Args>(args)...);
  }
}

#endif //!HG_COLT_PRINT