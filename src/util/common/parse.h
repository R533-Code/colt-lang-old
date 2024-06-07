/*****************************************************************//**
 * @file   parse.h
 * @brief  Contains the necessary code to parse strings (using scn).
 * 
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_PARSE
#define HG_COLT_PARSE

#include <charconv>
#include <concepts>
#include <limits>
#include <string_view>
#include "scn/scn.h"

#include "common/types.h"
#include "meta/meta_enum.h"
#include "meta/meta_maths.h"

namespace clt::meta
{
  template<Integral T>
  /// @brief The count of characters required to convert the greatest value
  /// of integer 'T' to string
  inline constexpr u64 max_digits10_v = static_cast<u64>(clt::ceil(clt::log10(std::numeric_limits<T>::max()))) + std::is_signed_v<T>;
}

DECLARE_ENUM_WITH_TYPE(u8, clt::io, IOError,
  FILE_EOF,
  FILE_ERROR,
  INVALID_ENCODING
);

DECLARE_ENUM_WITH_TYPE(u8, clt, ParsingCode,
  GOOD,             //no errors
  FILE_EOF,         //EOF detected
  FILE_ERROR,       //error reading from file
  INVALID_ENCODING, //invalid characters encountered (non-ASCII)
  EXPECTED_MORE,    //expected more characters
  INVALID_VALUE,    //invalid value to parse
  OUT_OF_RANGE,     //value cannot be stored in type
  NON_EMPTY_REM     //not all characters were consumed
);

namespace clt
{
  /// @brief The result of parsing a string
  class ParsingResult
  {
    /// @brief The error message or nullptr if no error
    const char* _msg = "No errors.";
    /// @brief The error code (ParsingCode::GOOD if no error)
    ParsingCode _code = ParsingCode::GOOD;

  public:
    /// @brief Default constructor
    constexpr ParsingResult() noexcept = default;
    /// @brief Default copy constructor
    constexpr ParsingResult(const ParsingResult&) noexcept = default;
    /// @brief Default move constructor
    constexpr ParsingResult(ParsingResult&&) noexcept = default;
    /// @brief Default copy assignment operator 
    constexpr ParsingResult& operator=(const ParsingResult&) noexcept = default;
    /// @brief Default move assignment operator 
    constexpr ParsingResult& operator=(ParsingResult&&) noexcept = default;
    
    /// @brief Constructs a result with 'code' and 'msg'
    /// @param code The error code (ParsingCode::GOOD if no error)
    /// @param msg The message describing the error
    constexpr ParsingResult(ParsingCode code, const char* msg) noexcept
      : _msg(msg), _code(code)
    {
      assert_true("Message cannot be nullptr!", msg != nullptr);
    }

    /// @brief The error code (ParsingCode::GOOD if no error)
    /// @return The error code
    constexpr ParsingCode code() const noexcept { return _code; }
    /// @brief The message describing the error or nullptr if code() is GOOD
    /// @return The message or nullptr
    constexpr const char* msg() const noexcept { return _msg; }

    explicit constexpr operator bool() const noexcept
    {
      return _code == ParsingCode::GOOD;
    }

    constexpr bool operator!() const noexcept
    {
      return _code != ParsingCode::GOOD;
    }

    constexpr bool operator==(const ParsingCode& code) const noexcept
    {
      return _code == code;
    }

    constexpr bool operator!=(const ParsingCode& code) const noexcept
    {
      return _code != code;
    }
  };

  namespace details
  {
    /// @brief Converts a scn::error to a ParsingResult
    /// @param code The error to convert
    /// @return ParsingResult representing the error
    constexpr ParsingResult scn_error_to_ParsingResult(scn::error code) noexcept
    {
      using enum clt::ParsingCode;
      switch_no_default(code.code())
      {
      case scn::error::good:
        return { GOOD, "No errors." };
      case scn::error::end_of_range:
        return { EXPECTED_MORE, code.msg() };
      case scn::error::invalid_scanned_value:
        return { INVALID_VALUE, code.msg() };
      case scn::error::value_out_of_range:
        return { OUT_OF_RANGE, code.msg() };
      case scn::error::invalid_encoding:
        return { INVALID_ENCODING, code.msg() };
      }
    }

    /// @brief Converts an IOError to a ParsingResult
    /// @param err The error to convert
    /// @return ParsingResult representing the error
    constexpr ParsingResult IOError_to_ParsingResult(io::IOError err) noexcept
    {
      switch_no_default(err)
      {
      case clt::io::IOError::FILE_EOF:
        return { ParsingCode::FILE_EOF,     "End of file reached!" };
      case clt::io::IOError::FILE_ERROR:
        return { ParsingCode::FILE_ERROR,   "Error reading from file!" };
      case clt::io::IOError::INVALID_ENCODING:
        return { ParsingCode::FILE_ERROR,   "Invalid character encoding!" };
      }
    }
  }

  template<meta::Parsable T>
  /// @brief Scans a value from a StringView
  /// @param strv The string from which to parse
  /// @param value Where to write a valid result
  /// @return Parsing result
  ParsingResult parse(std::string_view strv, T& value) noexcept
  {
    return details::scn_error_to_ParsingResult(scn::scan_default(strv, value).error());
  }

  template<meta::Parsable T>
  /// @brief Scans a value from a StringView
  /// @param strv The string from which to parse
  /// @param fmt The format specification
  /// @param value Where to write a valid result
  /// @return Parsing result
  ParsingResult parse(std::string_view strv, std::string_view fmt, T& value) noexcept
  {
    return details::scn_error_to_ParsingResult(scn::scan(strv, fmt, value));
  }
}

template<>
struct fmt::formatter<clt::ParsingResult>
{
  bool human_readable = false;

  template<typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    auto it = ctx.begin();
    auto end = ctx.end();
    if (it == end)
      return it;
    if (*it == 'h')
    {
      ++it;
      human_readable = true;
    }
    assert_true("Possible format for ParsingResult are: {} or {:h}!", *it == '}');
    return it;
  }

  template<typename FormatContext>
  auto format(const clt::ParsingResult& vec, FormatContext& ctx)
  {
    using namespace clt;
    if (human_readable)
      return fmt::format_to(ctx.out(), "{}", vec.msg());
    return fmt::format_to(ctx.out(), "({:h}) {}", vec.code(), vec.msg());
  }
};

#endif //!HG_COLT_PARSE