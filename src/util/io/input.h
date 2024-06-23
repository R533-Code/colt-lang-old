/*****************************************************************/ /**
 * @file   input.h
 * @brief  Contains input<>() for getting input from the console.
 *
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_INPUT
#define HG_COLT_INPUT

#include <utility>

#include "io/parse.h"
#include "structs/string.h"
#include "print.h"
#include "meta/meta_enum.h"

namespace clt::io
{
  /// @brief Turns on/off echo to the console
  void toggle_echo() noexcept;

  /// @brief Prints 'Press any key to continue...' and waits for any key input.
  inline void press_to_continue() noexcept
  {
    std::fputs("Press any key to continue...", stdout);
    toggle_echo();
    (void)getchar();
    toggle_echo();
    std::fputc('\n', stdout);
  }

  template<typename... Args>
  /// @brief Extracts a line from an opened file
  /// @param file The opened file from which to read
  /// @param fmt The format of the message to write
  /// @param args The arguments to format to the message
  /// @return The line or the encountered error
  inline Expect<String, ParsingResult> getline(
      std::FILE* file, fmt_str<Args...> fmt, Args&&... args) noexcept
  {
    //Print the message...
    print<"">(fmt, std::forward<Args>(args)...);
    //Ask for input...
    auto str = String::getLine(file);
    if (str.is_error()) //FILE_EOF or FILE_ERROR or INVALID_ENCODING
      return {Error, clt::details::IOError_to_ParsingResult(str.error())};
    return std::move(*str);
  }

  template<typename... Args>
  /// @brief Extracts a line from 'stdin'
  /// @param fmt The format of the message to write
  /// @param args The arguments to format to the message
  /// @return The line or the encountered error
  inline Expect<String, ParsingResult> getline(
      fmt_str<Args...> fmt, Args&&... args) noexcept
  {
    return getline(stdin, fmt, std::forward<Args>(args)...);
  }

  template<typename T = String, typename... Args>
    requires meta::Parsable<T>
  /// @brief Scans a line from an opened file, converting it to 'T'
  /// @tparam T The type to convert to
  /// @param file The opened file from which to read
  /// @param fmt The format of the message to write
  /// @param args The arguments to format to the message
  /// @return The parsed object or the encountered error
  inline Expect<T, ParsingResult> input(
      std::FILE* file, fmt_str<Args...> fmt, Args&&... args) noexcept
  {
    //Print the message...
    print<"">(fmt, std::forward<Args>(args)...);
    //Ask for input...
    auto str = String::getLine(file);
    if (str.is_error()) //FILE_EOF or FILE_ERROR or INVALID_ENCODING
      return {Error, clt::details::IOError_to_ParsingResult(str.error())};

    T ret;
    StringView strv = *str;
    auto result     = scn::scan_default(strip(strv), ret);
    if (result)
    {
      if (!result.empty())
        return {
            Error,
            ParsingResult{
                ParsingCode::NON_EMPTY_REM, "Not all characters were consumed!"}};
      return std::move(ret);
    }
    return {Error, clt::details::scn_error_to_ParsingResult(result.error())};
  }

  template<typename T = String, meta::StringLiteral endl = "", typename... Args>
  /// @brief Scans a line from 'stdin', converting it to 'T'
  /// @tparam T The type to convert to
  /// @param fmt The format of the message to write
  /// @param args The arguments to format to the message
  /// @return The parsed object or the encountered error
  inline Expect<T, ParsingResult> input(
      fmt_str<Args...> fmt, Args&&... args) noexcept
  {
    return input<T>(stdin, fmt, std::forward<Args>(args)...);
  }
} // namespace clt::io

#endif //!HG_COLT_INPUT