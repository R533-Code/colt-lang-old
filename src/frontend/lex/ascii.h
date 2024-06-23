/*****************************************************************/ /**
 * @file   ascii.h
 * @brief  Contains helpers to identify the category of ASCII character.
 *
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_ASCII
#define HG_COLT_ASCII

#include <string_view>
#include "common/types.h"
#include <ranges>

namespace clt
{
  namespace details
  {
    /// @brief Used to extract data from CHAR_INFO_TABLE (see example in 'islower')
    enum CharInfo : u8
    {
      /// @brief Check if the result of indexing into
      /// CHAR_INFO_TABLE is a control character.
      ISCNTRL = (1 << 0),
      /// @brief Check if the result of indexing into
      /// CHAR_INFO_TABLE is a digit character.
      ISDIGIT = (1 << 2),
      /// @brief Check if the result of indexing into
      /// CHAR_INFO_TABLE is a lower case letter.
      ISLOWER = (1 << 3),
      /// @brief Check if the result of indexing into
      /// CHAR_INFO_TABLE is a punctuation character.
      ISPUNCT = (1 << 4),
      /// @brief Check if the result of indexing into
      /// CHAR_INFO_TABLE is a whitespace character.
      ISSPACE = (1 << 5),
      /// @brief Check if the result of indexing into
      /// CHAR_INFO_TABLE is an upper case letter.
      ISUPPER = (1 << 6),

      /// @brief Check if the result of indexing into
      /// CHAR_INFO_TABLE is a letter.
      ISALPHA = ISUPPER | ISLOWER,
      /// @brief Check if the result of indexing into
      /// CHAR_INFO_TABLE is an alpha-numeric character.
      ISALNUM = ISALPHA | ISDIGIT,
      /// @brief Check if the result of indexing into
      /// CHAR_INFO_TABLE is a graphical character.
      ISGRAPH = ISALNUM | ISPUNCT,
    };

    /// @brief Table of all the characters information
    constexpr std::array<u8, 256> CHAR_INFO_TABLE = {
        0b00000001, //[NULL]
        0b00000001, //[START OF HEADING]
        0b00000001, //[START OF TEXT]
        0b00000001, //[END OF TEXT]
        0b00000001, //[END OF TRANSMISSION]
        0b00000001, //[ENQUIRY]
        0b00000001, //[ACKNOWLEDGE]
        0b00000001, //[BELL]
        0b00000001, //[BACKSPACE]
        0b00100001, //[HORIZONTAL TAB]
        0b00100001, //[LINE FEED]
        0b00100001, //[VERTICAL TAB]
        0b00100001, //[FORM FEED]
        0b00100001, //[CARRIAGE RETURN]
        0b00000001, //[SHIFT OUT]
        0b00000001, //[SHIFT IN]
        0b00000001, //[DATA LINK ESCAPE]
        0b00000001, //[DEVICE CONTROL 1]
        0b00000001, //[DEVICE CONTROL 2]
        0b00000001, //[DEVICE CONTROL 3]
        0b00000001, //[DEVICE CONTROL 4]
        0b00000001, //[NEGATIVE ACKNOWLEDGE]
        0b00000001, //[SYNCHRONOUS IDLE]
        0b00000001, //[ENG OF TRANS. BLOCK]
        0b00000001, //[CANCEL]
        0b00000001, //[END OF MEDIUM]
        0b00000001, //[SUBSTITUTE]
        0b00000001, //[ESCAPE]
        0b00000001, //[FILE SEPARATOR]
        0b00000001, //[GROUP SEPARATOR]
        0b00000001, //[RECORD SEPARATOR]
        0b00000001, //[UNIT SEPARATOR]
        0b00100000, //[SPACE]
        0b00010000, // '!'
        0b00010000, // '"'
        0b00010000, // '#'
        0b00010000, // '$'
        0b00010000, // '%'
        0b00010000, // '&'
        0b00010000, // '''
        0b00010000, // '('
        0b00010000, // ')'
        0b00010000, // '*'
        0b00010000, // '+'
        0b00010000, // ','
        0b00010000, // '-'
        0b00010000, // '.'
        0b00010000, // '/'
        0b00000100, // '0'
        0b00000100, // '1'
        0b00000100, // '2'
        0b00000100, // '3'
        0b00000100, // '4'
        0b00000100, // '5'
        0b00000100, // '6'
        0b00000100, // '7'
        0b00000100, // '8'
        0b00000100, // '9'
        0b00010000, // ':'
        0b00010000, // ';'
        0b00010000, // '<'
        0b00010000, // '='
        0b00010000, // '>'
        0b00010000, // '?'
        0b00010000, // '@'
        0b01000000, // 'A'
        0b01000000, // 'B'
        0b01000000, // 'C'
        0b01000000, // 'D'
        0b01000000, // 'E'
        0b01000000, // 'F'
        0b01000000, // 'G'
        0b01000000, // 'H'
        0b01000000, // 'I'
        0b01000000, // 'J'
        0b01000000, // 'K'
        0b01000000, // 'L'
        0b01000000, // 'M'
        0b01000000, // 'N'
        0b01000000, // 'O'
        0b01000000, // 'P'
        0b01000000, // 'Q'
        0b01000000, // 'R'
        0b01000000, // 'S'
        0b01000000, // 'T'
        0b01000000, // 'U'
        0b01000000, // 'V'
        0b01000000, // 'W'
        0b01000000, // 'X'
        0b01000000, // 'Y'
        0b01000000, // 'Z'
        0b00010000, // '['
        0b00010000, // '\'
        0b00010000, // ']'
        0b00010000, // '^'
        0b00010000, // '_'
        0b00010000, // '`'
        0b00001000, // 'a'
        0b00001000, // 'b'
        0b00001000, // 'c'
        0b00001000, // 'd'
        0b00001000, // 'e'
        0b00001000, // 'f'
        0b00001000, // 'g'
        0b00001000, // 'h'
        0b00001000, // 'i'
        0b00001000, // 'j'
        0b00001000, // 'k'
        0b00001000, // 'l'
        0b00001000, // 'm'
        0b00001000, // 'n'
        0b00001000, // 'o'
        0b00001000, // 'p'
        0b00001000, // 'q'
        0b00001000, // 'r'
        0b00001000, // 's'
        0b00001000, // 't'
        0b00001000, // 'u'
        0b00001000, // 'v'
        0b00001000, // 'w'
        0b00001000, // 'x'
        0b00001000, // 'y'
        0b00001000, // 'z'
        0b00010000, // '{'
        0b00010000, // '|'
        0b00010000, // '}'
        0b00010000, // '~'
        0b00000001, //[DEL]
    };
  } // namespace details

  /// @brief Checks if the given character is a control character.
  /// Locale independent.
  /// @param chr The character to check
  /// @return True if the character is a control character
  constexpr bool iscntrl(char chr) noexcept
  {
    using namespace details;
    return CHAR_INFO_TABLE[static_cast<u8>(chr)] & CharInfo::ISCNTRL;
  }

  /// @brief Checks if the given character is a letter.
  /// Locale independent.
  /// @param chr The character to check
  /// @return True if the character is a letter
  constexpr bool isalpha(char chr) noexcept
  {
    using namespace details;
    return CHAR_INFO_TABLE[static_cast<u8>(chr)] & CharInfo::ISALPHA;
  }

  /// @brief Checks if the given character is a digit (0-9).
  /// Locale independent.
  /// @param chr The character to check
  /// @return True if the character is a digit
  constexpr bool isdigit(char chr) noexcept
  {
    using namespace details;
    return CHAR_INFO_TABLE[static_cast<u8>(chr)] & CharInfo::ISDIGIT;
  }

  /// @brief Checks if the given character is a letter or digit.
  /// Locale independent.
  /// @param chr The character to check
  /// @return True if the character is an alpha-numeric character
  constexpr bool isalnum(char chr) noexcept
  {
    using namespace details;
    return CHAR_INFO_TABLE[static_cast<u8>(chr)] & CharInfo::ISALNUM;
  }

  /// @brief Checks if the given character is an lower case letter.
  /// Locale independent.
  /// @param chr The character to check
  /// @return True if the character is an lower case letter
  constexpr bool islower(char chr) noexcept
  {
    using namespace details;
    return CHAR_INFO_TABLE[static_cast<u8>(chr)] & CharInfo::ISLOWER;
  }

  /// @brief Checks if the given character is an upper case letter.
  /// Locale independent.
  /// @param chr The character to check
  /// @return True if the character is an upper case letter
  constexpr bool isupper(char chr) noexcept
  {
    using namespace details;
    return CHAR_INFO_TABLE[static_cast<u8>(chr)] & CharInfo::ISUPPER;
  }

  /// @brief Checks if the given character is any of !\"\#\$\%\&'()*+,-\./:;\<\=\>?\@[\\]^_`\{\|\}\~.
  /// Locale independent.
  /// @param chr The character to check
  /// @return True if the character is a punctuation character
  constexpr bool ispunct(char chr) noexcept
  {
    using namespace details;
    return CHAR_INFO_TABLE[static_cast<u8>(chr)] & CharInfo::ISPUNCT;
  }

  /// @brief Checks if the given character has a graphical representation.
  /// Locale independent.
  /// @param chr The character to check
  /// @return True if the character is graphic
  constexpr bool isgraph(char chr) noexcept
  {
    using namespace details;
    return CHAR_INFO_TABLE[static_cast<u8>(chr)] & CharInfo::ISGRAPH;
  }

  /// @brief Checks if a character is a ' ', '\\f', '\\n', '\\r', '\\t' or '\\v'.
  /// Locale independent.
  /// @param chr The character to check
  /// @return True if the character is whitespace
  constexpr bool isspace(char chr) noexcept
  {
    using namespace details;
    return CHAR_INFO_TABLE[static_cast<u8>(chr)] & CharInfo::ISSPACE;
  }

  /// @brief Checks if a character is a space or horizontal tab.
  /// Locale independent.
  /// @param chr The character to check
  /// @return True if space or horizontal tab
  constexpr bool isblank(char chr) noexcept
  {
    //Most compiler optimize '||' to an OR instruction in
    //this case (to avoid branching), but do it anyway...
    return static_cast<u8>(chr == ' ') | static_cast<u8>(chr == '\t');
  }

  /// @brief Converts a character to its upper case equivalent only if it is a lower case letter.
  /// Locale independent.
  /// @param chr The character to convert
  /// @return Upper case equivalent if 'chr' is lower case, else 'chr'
  constexpr char toupper(char chr) noexcept
  {
    //Most compiler would compile the code into a branch-less equivalent
    //if we made use of an 'if' statement, but do it anyway...
    //The XOR allows us to turn off the bit that makes the letter lower case.
    //The multiplication forces the mask to be 0 (thus not affecting 'chr')
    //if 'chr' is a not lower case letter.
    return chr ^ (0b00100000 * static_cast<u8>(islower(chr)));
  }

  /// @brief Converts a character to its lower case equivalent only if it is an upper case letter.
  /// Locale independent.
  /// @param chr The character to convert
  /// @return Lower case equivalent if 'chr' is upper case, else 'chr'
  constexpr char tolower(char chr) noexcept
  {
    //Most compiler would compile the code into a branch-less equivalent
    //if we made use of an 'if' statement, but do it anyway...
    //To see explanation, look at: 'clt::toupper'.
    return chr | (0b00100000 * static_cast<u8>(isupper(chr)));
  }

  /// @brief Insensitive-case string comparison
  /// @param a The first string
  /// @param b The second string
  /// @return True if both strings are equal (without regard to case)
  constexpr bool is_equal_case_insensitive(
      std::string_view a, std::string_view b) noexcept
  {
    if (a.size() != b.size())
      return false;
    for (size_t i = 0; i < a.size(); i++)
      if (clt::tolower(a[i]) != clt::tolower(b[i]))
        return false;

    return true;
  }

  /// @brief Returns an iterator that splits a string using 'chr' as separator
  /// @param strv The string to split
  /// @param chr The separator
  /// @return Split iterator
  constexpr auto split_by_char(std::string_view strv, char chr) noexcept
  {
    return strv | std::views::split(chr)
           | std::views::transform(
               [](auto&& r) -> std::string_view {
                 return {r.begin(), r.end()};
               });
  }

  /// @brief Strips chars from the beginning and end of a StringView
  /// @param strv The StringView to strip
  /// @param fn The filter function (pops the character if it returns true)
  /// @return Stripped StringView
  constexpr std::string_view strip(
      std::string_view strv, bool (*fn)(char) = &clt::isspace) noexcept
  {
    while (!strv.empty() && fn(strv.front()))
      strv.remove_prefix(1);
    while (!strv.empty() && fn(strv.back()))
      strv.remove_suffix(1);
    return strv;
  }
} // namespace clt

#endif //!HG_COLT_ASCII