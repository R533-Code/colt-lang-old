/*****************************************************************//**
 * @file   sizes.h
 * @brief  Contains byte sizes units.
 * 
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_SIZES
#define HG_COLT_SIZES

#include <ratio>
#include <fmt/format.h>
#include <scn/scn.h>

#include "meta/meta_traits.h"
#include "common/types.h"
#include "lex/ascii.h"

namespace clt
{
  namespace meta
  {
    template<typename T>
    /// @brief True if the type represents a compile-time ratio
    concept ConstexprRatio = (std::is_integral_v<decltype(T::num)>) && (std::is_integral_v<decltype(T::den)>);
  }

  /// @brief Byte ratio
  using B         = std::ratio<1, 1>;
  /// @brief Byte ratio
  using Byte      = std::ratio<1, 1>;
  /// @brief Kibibyte ratio
  using KiB       = std::ratio<1024, 1>;
  /// @brief Kibibyte ratio
  using KibiByte  = std::ratio<1024, 1>;
  /// @brief Mebibyte ratio
  using MiB       = std::ratio<1024 * 1024, 1>;
  /// @brief Mebibyte ratio
  using MebiByte  = std::ratio<1024 * 1024, 1>;
  /// @brief Gibibyte ratio
  using GiB       = std::ratio<1024 * 1024 * 1024, 1>;
  /// @brief Gibibyte ratio
  using GibiByte  = std::ratio<1024 * 1024 * 1024, 1>;  

  template<meta::ConstexprRatio RatioT = B>
  /// @brief Class responsible of holding byte sizes
  /// @tparam RatioT The ratio to bytes
  struct ByteSize
  {
    /// @brief The count of bytes / RatioT::num
    u64 count = 0;

    /// @brief The ratio to byte of the size
    using ratio = RatioT;

    /// @brief Constructs a size of 0
    constexpr ByteSize() noexcept = default;

    /// @brief Constructs a size from a count of RatioT
    /// @param count The size
    constexpr ByteSize(u64 count) noexcept
      : count(count) {}

    template<typename rt> requires (RatioT::num < rt::num)
    /// @brief Constructs a size from another size only when the conversion is not lossy.
    /// Use size_cast for lossy conversions.
    /// @tparam rt The ratio of the other size
    /// @param count The other size
    constexpr ByteSize(ByteSize<rt> count) noexcept
      : count(count.count * rt::num) {}

    /// @brief Converts the size to a byte count
    /// @return Byte count
    constexpr u64 to_bytes() const noexcept { return count * RatioT::num; }

    /// @brief The value of the size
    /// @return The value of the size
    constexpr u64 value() const noexcept { return count; }

    template<typename rt>
    /// @brief Comparison operator==
    /// @tparam rt The ratio of the other size
    /// @param other The other size
    /// @return True if equal
    constexpr bool operator==(ByteSize<rt> other) const noexcept
    {
      //We would like to avoid overflow, so rather than comparing
      // to_bytes(), we convert to the biggest size.
      if constexpr (rt::num > RatioT::num)
        return other.count * rt::num / RatioT::num == count;
      else
        return count * RatioT::num / rt::num == other.count;
    }

    template<typename rt>
    /// @brief Comparison operator!=
    /// @tparam rt The ratio of the other size
    /// @param other The other size
    /// @return True if not equal
    constexpr bool operator!=(ByteSize<rt> other) const noexcept
    {
      if constexpr (rt::num > RatioT::num)
        return other.count * rt::num / RatioT::num != count;
      else
        return count * RatioT::num / rt::num != other.count;
    }

    template<typename rt>
    /// @brief Comparison operator<=
    /// @tparam rt The ratio of the other size
    /// @param other The other size
    /// @return True if less or equal
    constexpr bool operator<=(ByteSize<rt> other) const noexcept
    {
      if constexpr (rt::num > RatioT::num)
        return other.count * rt::num / RatioT::num <= count;
      else
        return count * RatioT::num / rt::num <= other.count;
    }

    template<typename rt>
    /// @brief Comparison operator>=
    /// @tparam rt The ratio of the other size
    /// @param other The other size
    /// @return True if greater or equal
    constexpr bool operator>=(ByteSize<rt> other) const noexcept
    {
      if constexpr (rt::num > RatioT::num)
        return other.count * rt::num / RatioT::num >= count;
      else
        return count * RatioT::num / rt::num >= other.count;
    }

    template<typename rt>
    /// @brief Comparison operator<
    /// @tparam rt The ratio of the other size
    /// @param other The other size
    /// @return True if less
    constexpr bool operator<(ByteSize<rt> other) const noexcept
    {
      if constexpr (rt::num > RatioT::num)
        return other.count * rt::num / RatioT::num < count;
      else
        return count * RatioT::num / rt::num < other.count;
    }

    template<typename rt>
    /// @brief Comparison operator>
    /// @tparam rt The ratio of the other size
    /// @param other The other size
    /// @return True if greater
    constexpr bool operator>(ByteSize<rt> other) const noexcept
    {
      if constexpr (rt::num > RatioT::num)
        return other.count * rt::num / RatioT::num > count;
      else
        return count * RatioT::num / rt::num > other.count;
    }

    template<meta::ConstexprRatio U>
    friend struct size;
  };

  template<meta::ConstexprRatio Ratio, meta::ConstexprRatio From>
  /// @brief Converts from a size type to another one (performing lossy conversions)
  /// @tparam Ratio The ratio to convert to
  /// @tparam From The ratio to convert from
  /// @param value The value to convert
  /// @return The converted value
  constexpr ByteSize<Ratio> size_cast(ByteSize<From> value) noexcept
  {
    return (value.value() * From::num) / Ratio::num;
  }
  
  /// @brief Creates a Byte size
  /// @param i The size count
  /// @return Byte size
  consteval ByteSize<B>   operator""_B(unsigned long long int i)   noexcept { return ByteSize<B>(i); }
  /// @brief Creates a Kibibyte size
  /// @param i The size count
  /// @return Kibibyte size
  consteval ByteSize<KiB> operator""_KiB(unsigned long long int i) noexcept { return ByteSize<KiB>(i); }
  /// @brief Creates a Mebibyte size
  /// @param i The size count
  /// @return Mebibyte size
  consteval ByteSize<MiB> operator""_MiB(unsigned long long int i) noexcept { return ByteSize<MiB>(i); }
  /// @brief Creates a Gibibyte size
  /// @param i The size count
  /// @return Gibibyte size
  consteval ByteSize<GiB> operator""_GiB(unsigned long long int i) noexcept { return ByteSize<GiB>(i); }
}

template<>
struct scn::scanner<clt::ByteSize<clt::B>>
  : scn::empty_parser
{
  template <typename Context>
  error scan(clt::ByteSize<clt::B>& val, Context& ctx)
  {
    using namespace clt;
    
    u64 count;
    std::string_view str;
    auto r = scn::scan(ctx.range(), "{}{}", count, str);
    ON_SCOPE_EXIT {
      ctx.range() = std::move(r.range());
    };

    if (!r)
    {
      if (r.error().code() == error::value_out_of_range)
        return r.error();
      return { r.error().code(), "Expected an integer followed by 'B', 'KiB', 'MiB', or 'GiB'." };
    }

    if (is_equal_case_insensitive(str, "B"))
    {
      val = ByteSize<B>{ count };
      return { error::good, nullptr };
    }
    if (is_equal_case_insensitive(str, "KiB"))
    {
      if (count > 18'014'398'509'481'984)
        return { error::value_out_of_range, "Value too great to be representable as bytes!" };
      val = ByteSize<KiB>{ count };
      return { error::good, nullptr };
    }
    if (is_equal_case_insensitive(str, "MiB"))
    {
      if (count > 17'592'186'044'416)
        return { error::value_out_of_range, "Value too great to be representable as bytes!" };
      val = ByteSize<MiB>{ count };
      return { error::good, nullptr };
    }
    if (is_equal_case_insensitive(str, "GiB"))
    {
      if (count > 17'179'869'184)
        return { error::value_out_of_range, "Value too great to be representable as bytes!" };
      val = ByteSize<GiB>{ count };
      return { error::good, nullptr };
    }
    return { error::invalid_scanned_value, "Expected 'B', 'KiB', 'MiB', or 'GiB'." };
  }
};

template<>
struct fmt::formatter<clt::ByteSize<clt::Byte>>
  : public clt::meta::DefaultParserFMT
{
  template<typename FormatContext>
  auto format(const clt::ByteSize<clt::Byte>& vec, FormatContext& ctx)
  {
    using namespace clt;
    if (vec.count % GiB::num == 0)
      return fmt::format_to(ctx.out(), "{}GiB", vec.count / GiB::num);
    if (vec.count % MiB::num == 0)
      return fmt::format_to(ctx.out(), "{}MiB", vec.count / MiB::num);
    if (vec.count % KiB::num == 0)
      return fmt::format_to(ctx.out(), "{}KiB", vec.count / KiB::num);
    return fmt::format_to(ctx.out(), "{}B", vec.count);
  }
};

template<>
struct fmt::formatter<clt::ByteSize<clt::KibiByte>>
  : public clt::meta::DefaultParserFMT
{
  template<typename FormatContext>
  auto format(const clt::ByteSize<clt::KibiByte>& vec, FormatContext& ctx)
  {
    using namespace clt;
    if (vec.count % GiB::num == 0)
      return fmt::format_to(ctx.out(), "{}GiB", vec.count / GiB::num);
    if (vec.count % MiB::num == 0)
      return fmt::format_to(ctx.out(), "{}MiB", vec.count / MiB::num);
    return fmt::format_to(ctx.out(), "{}KiB", vec.count / KiB::num);
  }
};

template<>
struct fmt::formatter<clt::ByteSize<clt::MebiByte>>
  : public clt::meta::DefaultParserFMT
{
  template<typename FormatContext>
  auto format(const clt::ByteSize<clt::MebiByte>& vec, FormatContext& ctx)
  {
    using namespace clt;
    if (vec.count % GiB::num == 0)
      return fmt::format_to(ctx.out(), "{}GiB", vec.count / GiB::num);
    return fmt::format_to(ctx.out(), "{}MiB", vec.count / MiB::num);
  }
};

template<>
struct fmt::formatter<clt::ByteSize<clt::GibiByte>>
  : public clt::meta::DefaultParserFMT
{
  template<typename FormatContext>
  auto format(const clt::ByteSize<clt::GibiByte>& vec, FormatContext& ctx)
  {
    using namespace clt;
    return fmt::format_to(ctx.out(), "{}GiB", vec.count / GiB::num);
  }
};

#endif //!HG_COLT_SIZES