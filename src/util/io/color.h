/*****************************************************************/ /**
 * @file   color.h
 * @brief  Contains ANSI escape codes to alter the color of the console.
 *
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_CONSOLE_COLORS
#define HG_COLT_CONSOLE_COLORS

#include <cstdint>
#include <array>

#include <fmt/format.h>

namespace clt::io
{
  namespace details
  {
    /// @brief Array of colors
    static constexpr std::array CONSOLE_COLORS = {
        "",         //EMPTY
        "\x1B[30m", //CONSOLE_FOREGROUND_BLACK
        "\x1B[31m", //CONSOLE_FOREGROUND_RED
        "\x1B[32m", //CONSOLE_FOREGROUND_GREEN
        "\x1B[33m", //CONSOLE_FOREGROUND_YELLOW
        "\x1B[34m", //CONSOLE_FOREGROUND_BLUE
        "\x1B[35m", //CONSOLE_FOREGROUND_MAGENTA
        "\x1B[36m", //CONSOLE_FOREGROUND_CYAN
        "\x1B[37m", //CONSOLE_FOREGROUND_WHITE

        "\x1B[90m", //CONSOLE_FOREGROUND_BRIGHT_BLACK
        "\x1B[91m", //CONSOLE_FOREGROUND_BRIGHT_RED
        "\x1B[92m", //CONSOLE_FOREGROUND_BRIGHT_GREEN
        "\x1B[93m", //CONSOLE_FOREGROUND_BRIGHT_YELLOW
        "\x1B[94m", //CONSOLE_FOREGROUND_BRIGHT_BLUE
        "\x1B[95m", //CONSOLE_FOREGROUND_BRIGHT_MAGENTA
        "\x1B[96m", //CONSOLE_FOREGROUND_BRIGHT_CYAN
        "\x1B[97m", //CONSOLE_FOREGROUND_BRIGHT_WHITE

        "\x1B[40m", //CONSOLE_BACKGROUND_BLACK
        "\x1B[41m", //CONSOLE_BACKGROUND_RED
        "\x1B[42m", //CONSOLE_BACKGROUND_GREEN
        "\x1B[43m", //CONSOLE_BACKGROUND_YELLOW
        "\x1B[44m", //CONSOLE_BACKGROUND_BLUE
        "\x1B[45m", //CONSOLE_BACKGROUND_MAGENTA
        "\x1B[46m", //CONSOLE_BACKGROUND_CYAN
        "\x1B[47m", //CONSOLE_BACKGROUND_WHITE

        "\x1B[100m", //CONSOLE_BACKGROUND_BRIGHT_BLACK
        "\x1B[101m", //CONSOLE_BACKGROUND_BRIGHT_RED
        "\x1B[102m", //CONSOLE_BACKGROUND_BRIGHT_GREEN
        "\x1B[103m", //CONSOLE_BACKGROUND_BRIGHT_YELLOW
        "\x1B[104m", //CONSOLE_BACKGROUND_BRIGHT_BLUE
        "\x1B[105m", //CONSOLE_BACKGROUND_BRIGHT_MAGENTA
        "\x1B[106m", //CONSOLE_BACKGROUND_BRIGHT_CYAN
        "\x1B[107m", //CONSOLE_BACKGROUND_BRIGHT_WHITE

        "\x1B[0m", //CONSOLE_COLOR_RESET
        "\x1B[2m", //CONSOLE_FONT_BOLD
        "\x1B[4m", //CONSOLE_FONT_UNDERLINE
        "\x1B[5m", //CONSOLE_FONT_FLICKER
        "\x1B[7m", //CONSOLE_COLOR_REVERSE
    };
  } // namespace details

  /// @brief If true, then colored output is used
  inline bool OutputColor = true;

  /// @brief Represents a Console Color
  struct Color
  {
    /// @brief Index into CONSOLE_COLOR array
    uint64_t index;
  };

  /// @brief Black foreground
  static constexpr Color BlackF = Color{1};
  /// @brief Red foreground
  static constexpr Color RedF = Color{2};
  /// @brief Green foreground
  static constexpr Color GreenF = Color{3};
  /// @brief Yellow foreground
  static constexpr Color YellowF = Color{4};
  /// @brief Blue foreground
  static constexpr Color BlueF = Color{5};
  /// @brief Magenta foreground
  static constexpr Color MagentaF = Color{6};
  /// @brief Cyan foreground
  static constexpr Color CyanF = Color{7};
  /// @brief White foreground (usually the default)
  static constexpr Color WhiteF = Color{8};

  /// @brief Bright Black foreground
  static constexpr Color BrightBlackF = Color{9};
  /// @brief Bright Red foreground
  static constexpr Color BrightRedF = Color{10};
  /// @brief Bright Green foreground
  static constexpr Color BrightGreenF = Color{11};
  /// @brief Bright Yellow foreground
  static constexpr Color BrightYellowF = Color{12};
  /// @brief Bright Blue foreground
  static constexpr Color BrightBlueF = Color{13};
  /// @brief Bright Magenta foreground
  static constexpr Color BrightMagentaF = Color{14};
  /// @brief Bright Cyan foreground
  static constexpr Color BrightCyanF = Color{15};
  /// @brief Bright White foreground
  static constexpr Color BrightWhiteF = Color{16};

  /// @brief Black background
  static constexpr Color BlackB = Color{17};
  /// @brief Red background
  static constexpr Color RedB = Color{18};
  /// @brief Green background
  static constexpr Color GreenB = Color{19};
  /// @brief Yellow background
  static constexpr Color YellowB = Color{20};
  /// @brief Blue background
  static constexpr Color BlueB = Color{21};
  /// @brief Magenta background
  static constexpr Color MagentaB = Color{22};
  /// @brief Cyan background
  static constexpr Color CyanB = Color{23};
  /// @brief White background
  static constexpr Color WhiteB = Color{24};

  /// @brief Bright Black background
  static constexpr Color BrightBlackB = Color{25};
  /// @brief Bright Red background
  static constexpr Color BrightRedB = Color{26};
  /// @brief Bright Green background
  static constexpr Color BrightGreenB = Color{27};
  /// @brief Bright Yellow background
  static constexpr Color BrightYellowB = Color{28};
  /// @brief Bright Blue background
  static constexpr Color BrightBlueB = Color{29};
  /// @brief Bright Magenta background
  static constexpr Color BrightMagentaB = Color{30};
  /// @brief Bright Cyan background
  static constexpr Color BrightCyanB = Color{31};
  /// @brief Bright White background
  static constexpr Color BrightWhiteB = Color{32};

  /// @brief Reset foreground and background color to default
  static constexpr Color Reset = Color{33};
  /// @brief Bold font
  static constexpr Color Bold = Color{34};
  /// @brief Underline
  static constexpr Color Underline = Color{35};
  /// @brief Flicker
  static constexpr Color Flicker = Color{36};
  /// @brief Switch foreground and background color
  static constexpr Color SwitchFB = Color{37};
} // namespace clt::io

template<>
/// @brief {fmt} specialization of Color
struct fmt::formatter<clt::io::Color>
{
  template<typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template<typename FormatContext>
  /// @brief fmt overload
  /// @tparam FormatContext The context to write
  /// @param op The BinaryOperator to write
  /// @param ctx The context
  /// @return context
  auto format(const clt::io::Color& op, FormatContext& ctx)
  {
    // If OutputColor is false, we write an empty string "".
    return fmt::format_to(
        ctx.out(), "{}",
        clt::io::details::CONSOLE_COLORS
            [op.index * static_cast<uint64_t>(clt::io::OutputColor)]);
  }
};

#endif //!HG_CONSOLE_COLORS