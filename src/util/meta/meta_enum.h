/*****************************************************************/ /**
 * @file   meta_enum.h
 * @brief  Contains helpers for dealing with enums and reflecting on them.
 *
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_ENUM
#define HG_COLT_ENUM

#include <array>
#include <utility>

#include "meta/meta_reflect.h"
#include "meta/meta_constexpr_map.h"
#include "common/macros.h"
#include "common/types.h"

namespace clt::iter
{
  template<u64 begin, u64 end>
  /// @brief Iterator from that returns value from [begin, end]
  class RangeIterator
  {
    /// @brief The beginning of the Range
    u64 current = begin;

  public:
    /// @brief Default constructor
    constexpr RangeIterator() noexcept = default;
    /// @brief Set the starting value to current
    /// @param current The starting value
    constexpr RangeIterator(u64 current) noexcept
        : current(current)
    {
    }

    /// @brief Reads the current value of the range
    /// @return The current value of the range
    constexpr u64 operator*() const noexcept { return current; }

    /// @brief Increments the iterator to the next value in the range
    /// @return Self
    constexpr RangeIterator& operator++() noexcept
    {
      ++current;
      return *this;
    }

    /// @brief Increments the iterator to the next value in the range, returning the old value
    /// @return Copy of old iterator
    constexpr RangeIterator operator++(int) noexcept
    {
      RangeIterator tmp = *this;
      ++(*this);
      return tmp;
    }

    /// @brief Decrements the iterator to the next value in the range
    /// @return Self
    constexpr RangeIterator& operator--() noexcept
    {
      --current;
      return *this;
    }

    /// @brief Decrements the iterator to the next value in the range, returning the old value
    /// @return Copy of old iterator
    constexpr RangeIterator operator--(int) noexcept
    {
      RangeIterator tmp = *this;
      --(*this);
      return tmp;
    }

    friend constexpr bool operator==(const RangeIterator&, const RangeIterator&) =
        default;
  };
  template<typename To, typename Iter>
  /// @brief Converts the return of an iterator to another type
  class ConvertIterator
  {
    /// @brief The internal iterator
    Iter iterator;

  public:
    /// @brief Default constructor
    constexpr ConvertIterator() noexcept = default;

    template<typename U, typename... Args>
      requires(!std::same_as<ConvertIterator, std::remove_cvref_t<U>>)
    constexpr ConvertIterator(U&& ref, Args&&... args) noexcept
        : iterator(std::forward<U>(ref), std::forward<Args>(args)...)
    {
    }

    /// @brief Reads the current value of the range
    /// @return The current value of the range
    constexpr To operator*() const noexcept { return static_cast<To>(*iterator); }

    /// @brief Increments the iterator to the next value in the range
    /// @return Self
    constexpr ConvertIterator& operator++() noexcept
    {
      ++iterator;
      return *this;
    }

    /// @brief Increments the iterator to the next value in the range, returning the old value
    /// @return Copy of old iterator
    constexpr ConvertIterator operator++(int) noexcept
    {
      ConvertIterator tmp = *this;
      ++(*this);
      return tmp;
    }

    /// @brief Decrements the iterator to the next value in the range
    /// @return Self
    constexpr ConvertIterator& operator--() noexcept
    {
      --iterator;
      return *this;
    }

    /// @brief Decrements the iterator to the next value in the range, returning the old value
    /// @return Copy of old iterator
    constexpr ConvertIterator operator--(int) noexcept
    {
      ConvertIterator tmp = *this;
      --(*this);
      return tmp;
    }

    friend constexpr bool operator==(
        const ConvertIterator&, const ConvertIterator&) = default;
  };

  template<typename Enum, u64 BEGIN, u64 END>
  /// @brief Iterator over an contiguous enum range
  struct EnumIter
  {
    /// @brief Returns the start of the enum range
    /// @return Start of the enum range
    constexpr ConvertIterator<Enum, RangeIterator<BEGIN, END>> begin() const noexcept
    {
      return {};
    }
    /// @brief Returns the end of the enum range
    /// @return End of the enum range
    constexpr ConvertIterator<Enum, RangeIterator<BEGIN, END>> end() const noexcept
    {
      return END + 1;
    }
  };
} // namespace clt::iter

#define COLT_DETAILS_EXPAND_ENUM(en) , en
#define COLT_DETAILS_STRINGIZE_ENUM(en) \
  , std::string_view                    \
  {                                     \
    #en                                 \
  }
#define COLT_DETAILS_MAP_PAIR_ENUM(en) \
  , std::pair                          \
  {                                    \
    std::string_view{#en}, en          \
  }

#define ADD_REFLECTION_FOR_CONSECUTIVE_ENUM(namespace_name, name, first, ...)                            \
  template<>                                                                                             \
  struct clt::meta::entity_kind<namespace_name::name>                                                    \
  {                                                                                                      \
    static constexpr clt::meta::EntityKind value = clt::meta::IS_ENUM;                                   \
  };                                                                                                     \
  template<>                                                                                             \
  struct clt::reflect<namespace_name::name>                                                              \
  {                                                                                                      \
    using enum_type = std::underlying_type_t<namespace_name::name>;                                      \
    static constexpr std::string_view str() noexcept                                                     \
    {                                                                                                    \
      return #name;                                                                                      \
    }                                                                                                    \
    static constexpr bool is_consecutive() noexcept                                                      \
    {                                                                                                    \
      return true;                                                                                       \
    }                                                                                                    \
    static constexpr std::array name##_str = {std::string_view{                                          \
        #first} COLT_FOR_EACH(COLT_DETAILS_STRINGIZE_ENUM, __VA_ARGS__)};                                \
    static constexpr size_t min() noexcept                                                               \
    {                                                                                                    \
      return 0;                                                                                          \
    }                                                                                                    \
    static constexpr size_t max() noexcept                                                               \
    {                                                                                                    \
      return name##_str.size() - 1;                                                                      \
    }                                                                                                    \
    static constexpr size_t count() noexcept                                                             \
    {                                                                                                    \
      return name##_str.size();                                                                          \
    }                                                                                                    \
    static constexpr std::string_view to_str(namespace_name::name value)                                 \
    {                                                                                                    \
      assert_true("Enum out of range!", static_cast<enum_type>(value) <= max());                         \
      return name##_str[static_cast<enum_type>(value)];                                                  \
    }                                                                                                    \
    static constexpr clt::Option<namespace_name::name> from(enum_type value)                             \
    {                                                                                                    \
      if (value > max())                                                                                 \
        return clt::None;                                                                                \
      return static_cast<namespace_name::name>(value);                                                   \
    }                                                                                                    \
                                                                                                         \
  private:                                                                                               \
    using ArrayTable_t = std::array<                                                                     \
        std::pair<std::string_view, namespace_name::name>, name##_str.size()>;                           \
    using Map_t = clt::meta::ConstexprMap<                                                               \
        std::string_view, namespace_name::name, name##_str.size()>;                                      \
    static constexpr ArrayTable_t get_array()                                                            \
    {                                                                                                    \
      using enum namespace_name::name;                                                                   \
      ArrayTable_t ret = {std::pair{std::string_view{#first}, first} COLT_FOR_EACH(                      \
          COLT_DETAILS_MAP_PAIR_ENUM, __VA_ARGS__)};                                                     \
      return ret;                                                                                        \
    }                                                                                                    \
    static const ArrayTable_t internal_map;                                                              \
    static const Map_t map;                                                                              \
                                                                                                         \
  public:                                                                                                \
    static constexpr clt::Option<namespace_name::name> from(std::string_view str)                        \
    {                                                                                                    \
      return map.find(str);                                                                              \
    }                                                                                                    \
    static constexpr clt::iter::EnumIter<namespace_name::name, 0, name##_str.size() - 1> iter() noexcept \
    {                                                                                                    \
      return {};                                                                                         \
    }                                                                                                    \
  };                                                                                                     \
  inline constexpr clt::reflect<namespace_name::name>::ArrayTable_t                                      \
      clt::reflect<namespace_name::name>::internal_map =                                                 \
          clt::reflect<namespace_name::name>::get_array();                                               \
  inline constexpr clt::reflect<namespace_name::name>::Map_t                                             \
      clt::reflect<namespace_name::name>::map = {                                                        \
          {clt::reflect<namespace_name::name>::internal_map}};                                           \
  template<>                                                                                             \
  struct clt::meta::is_reflectable<namespace_name::name>                                                 \
  {                                                                                                      \
    static constexpr bool value = true;                                                                  \
  }

/// @brief Declares an enumeration with reflection support
#define DECLARE_ENUM_WITH_TYPE(type, namespace_name, name, first, ...) \
  namespace namespace_name                                             \
  {                                                                    \
    enum class name : type                                             \
    {                                                                  \
      first COLT_FOR_EACH(COLT_DETAILS_EXPAND_ENUM, __VA_ARGS__)       \
    };                                                                 \
  }                                                                    \
  ADD_REFLECTION_FOR_CONSECUTIVE_ENUM(namespace_name, name, first, __VA_ARGS__)

#define DECLARE_ENUM(namespace_name, name, first, ...)           \
  namespace namespace_name                                       \
  {                                                              \
    enum class name                                              \
    {                                                            \
      first COLT_FOR_EACH(COLT_DETAILS_EXPAND_ENUM, __VA_ARGS__) \
    };                                                           \
  }                                                              \
  ADD_REFLECTION_FOR_CONSECUTIVE_ENUM(namespace_name, name, first, __VA_ARGS__)

template<typename T>
  requires std::is_enum_v<T> && clt::meta::is_reflectable_v<T>
struct fmt::formatter<T>
{
  bool human_readable = false;

  template<typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    auto it  = ctx.begin();
    auto end = ctx.end();
    if (it == end)
      return it;
    if (*it == 'h')
    {
      ++it;
      human_readable = true;
    }
    assert_true("Possible format for Enum are: {} or {:h}!", *it == '}');
    return it;
  }

  template<typename FormatContext>
  auto format(const T& exp, FormatContext& ctx)
  {
    using namespace clt;

    if (human_readable)
      return fmt::format_to(ctx.out(), "{}", reflect<T>::to_str(exp));
    return fmt::format_to(
        ctx.out(), "{}::{}", reflect<T>::str(), reflect<T>::to_str(exp));
  }
};

//Add reflection for already existing enum
ADD_REFLECTION_FOR_CONSECUTIVE_ENUM(
    clt::meta, EntityKind, IS_ENUM, IS_BUILTIN, IS_CLASS, IS_UNKNOWN);

#endif //!HG_COLT_ENUM