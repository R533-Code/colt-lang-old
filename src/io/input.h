///** @file input.h
//* Contains input<>() functions for getting input from the console.
//*/
//
//#ifndef HG_COLT_INPUT
//#define HG_COLT_INPUT
//
//#include <utility>
//
//#include "util/parse.h"
//#include "print.h"
//#include "meta/meta_enum.h"
//
//namespace clt::io
//{
//  /// @brief Turns on/off echo to the console
//  void toggle_echo() noexcept;  
//
//  /// @brief Prints 'Press any key to continue...' and waits for any key input.
//  inline void press_to_continue() noexcept
//  {
//    std::fputs("Press any key to continue...", stdout);
//    toggle_echo();
//    (void)getchar();
//    toggle_echo();
//    std::fputc('\n', stdout);
//  }
//
//  template<typename... Args>
//  inline Expect<String, ParsingResult> getline(std::FILE* file, fmt_str<Args...> fmt, Args&&... args) noexcept
//  {
//    //Print the message...
//    print<"">(fmt, std::forward<Args>(args)...);
//    //Ask for input...
//    auto str = String::getLine(file);
//    if (str.is_error()) //FILE_EOF or FILE_ERROR or INVALID_ENCODING
//      return { Error, clt::details::IOError_to_ParsingResult(str.error()) };
//    return std::move(*str);
//  }
//
//  template<typename... Args>
//  inline Expect<String, ParsingResult> getline(fmt_str<Args...> fmt, Args&&... args) noexcept
//  {
//    return getline(stdin, fmt, std::forward<Args>(args)...);
//  }
//
//  template<typename T = String, typename... Args> requires meta::Parsable<T>
//  inline Expect<T, ParsingResult> input(std::FILE* file, fmt_str<Args...> fmt, Args&&... args) noexcept
//  {
//    //Print the message...
//    print<"">(fmt, std::forward<Args>(args)...);
//    //Ask for input...
//    auto str = String::getLine(file);
//    if (str.is_error()) //FILE_EOF or FILE_ERROR or INVALID_ENCODING
//      return { Error, clt::details::IOError_to_ParsingResult(str.error()) };
//
//    T ret;
//    StringView strv = *str;
//    auto result = scn::scan_default(strv.strip(), ret);
//    if (result)
//    {
//      if (!result.empty())
//        return { Error, ParsingResult{ ParsingCode::NON_EMPTY_REM, "Not all characters were consumed!" } };
//      return std::move(ret);
//    }
//    return { Error, clt::details::scn_error_to_ParsingResult(result.error()) };
//  }
//
//  template<typename T = String, meta::StringLiteral endl = "", typename... Args>
//  inline Expect<T, ParsingResult> input(fmt_str<Args...> fmt, Args&&... args) noexcept
//  {
//    return input<T>(stdin, fmt, std::forward<Args>(args)...);
//  }
//}
//
//#endif //!HG_COLT_INPUT