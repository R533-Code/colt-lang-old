/*****************************************************************//**
 * @file   meta_string_literal.h
 * @brief  Contains a StringLiteral type that can be passed as template
 *         argument.
 * 
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_META_STRING_LITERAL
#define HG_META_STRING_LITERAL

#include <array>
#include <string_view>

namespace clt::meta
{
  template <size_t N>
  /// @brief Type to pass string literals as template parameters
  struct StringLiteral
  {
    /// @brief Constructs a StringLiteral from a literal string
    /// @param str The literal string
    constexpr StringLiteral(const char(&str)[N])
    {
      std::copy_n(str, N, value);
    }

    /// @brief Size of the literal string (without NUL terminator)
    /// @return Size of the literal string
    static constexpr size_t size() noexcept { return N - 1; }

    /// @brief The value of the literal string
    char value[N];
  };

  template <StringLiteral... Strs>
  /// @brief Concatenates StringView at compile time
  struct join
  {
    /// @brief Concatenate all the StringView and returns an array storing the result
    static constexpr auto impl() noexcept
    {
      constexpr std::size_t len = (Strs.size() + ... + 0);
      std::array<char, len + 1> arr{};
      auto append = [i = 0, &arr](auto const& s) mutable {
        for (size_t j = 0; j < s.size(); j++) arr[i++] = s.value[j];
        };
      (append(Strs), ...);
      arr[len] = '\0';
      return arr;
    }

    /// @brief Array of characters representing concatenated string
    static constexpr auto arr = impl();
    
    /// @brief Concatenation result
    static constexpr const char* value{ arr.data() };
  };
  
  template <StringLiteral... Strs>
  /// @brief Short-hand for join<...>::value
  static constexpr auto join_v = join<Strs...>::value;

  template<const std::string_view&... Strs>
  /// @brief Concatenates StringView at compile time
  struct join_strv
  {
    /// @brief Concatenate all the StringView and returns an array storing the result
    static constexpr auto impl() noexcept
    {
      constexpr std::size_t len = (Strs.size() + ... + 0);
      std::array<char, len + 1> arr{};
      auto append = [i = 0, &arr](const auto& s) mutable {
        for (size_t j = 0; j < s.size(); j++) arr[i++] = s.data()[j];
        };
      (append(Strs), ...);
      arr[len] = '\0';
      return arr;
    }

    /// @brief Array of characters representing concatenated string
    static constexpr auto arr = impl();
    /// @brief Concatenation result
    static constexpr const char* value{ arr.data() };
  };

  template<const std::string_view&... Strs>
  /// @brief Short-hand for join<...>::value
  static constexpr auto join_strv_v = join_strv<Strs...>::value;
}

#endif //!HG_META_STRING_LITERAL