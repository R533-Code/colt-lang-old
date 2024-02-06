/*****************************************************************//**
 * @file   string.h
 * @brief  Contains a String class that uses COLT allocators.
 * 
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_STRING
#define HG_COLT_STRING

#include <string_view>
#include "vector.h"
#include "expect.h"
#include "meta/meta_enum.h"
#include "common.h"
#include "util/parse.h"

namespace clt
{
  template<auto ALLOCATOR, StringEncoding ENCODING>
    requires meta::AllocatorScope<ALLOCATOR>
  /// @brief Unspecialized BasicString
  class BasicString {};

  template<auto ALLOCATOR>
    requires meta::AllocatorScope<ALLOCATOR>
  /// @brief ASCII BasicString
  class BasicString<ALLOCATOR, StringEncoding::ASCII>
    : public Vector<char, ALLOCATOR>
  {
    /// @brief The underlying vector providing storage
    using UnderlyingVector = Vector<char, ALLOCATOR>;

  public:
    template<typename AllocT> requires meta::LocalAllocator<ALLOCATOR>
    /// @brief Constructor for BasicString using a local allocator
    /// @param alloc The local allocator
    constexpr BasicString(AllocT& alloc) noexcept
      : BasicString::UnderlyingVector(alloc) {}

    /// @brief Default constructor for BasicString using a global allocator
    constexpr BasicString() noexcept requires meta::GlobalAllocator<ALLOCATOR> = default;

    template<typename AllocT>
    /// @brief Constructor for BasicString using a local allocator
    /// @param alloc The local allocator
    /// @param strv The StringView to use whose content to copy
    constexpr BasicString(AllocT& alloc, StringView strv) noexcept requires meta::LocalAllocator<ALLOCATOR>
      : BasicString::UnderlyingVector(alloc, strv) {}

    /// @brief Constructor for BasicString using a global allocator
    /// @param strv The StringView to use whose content to copy
    constexpr BasicString(StringView strv) noexcept requires meta::GlobalAllocator<ALLOCATOR>
      : BasicString::UnderlyingVector(strv) {}

    template<typename AllocT, size_t N> requires meta::LocalAllocator<ALLOCATOR>
    /// @brief Constructs a String
    /// @param alloc The local allocator
    /// @param x The array
    constexpr BasicString(AllocT& alloc, const char(&x)[N]) noexcept
      : BasicString::UnderlyingVector(alloc, StringView{ x, x + N }) {}

    template<size_t N> requires meta::GlobalAllocator<ALLOCATOR>
    /// @brief Constructs a String
    /// @param x The array
    constexpr BasicString(const char(&x)[N]) noexcept
      : BasicString::UnderlyingVector(StringView{ x, x + N }) {}

    /// @brief Converts a String to a StringView
    /// @return Span over the whole Vector
    constexpr operator StringView() const noexcept
    {
      return StringView{ this->begin(), this->end() };
    }

    /// @brief Pushes a StringView at the end of the String
    /// @param strv The StringView to push back
    /// @param repeat The number of times to repeat the operation
    constexpr BasicString& push_back(StringView strv, u64 repeat = 1) noexcept
    {
      for (size_t z = 0; z < repeat; z++)
        for (auto i : strv)
          BasicString::UnderlyingVector::push_back(i);
      return *this;
    }

    /// @brief Pushes a character at the end of the String
    /// @param i The character to push back
    /// @param repeat The number of times to repeat the operation
    constexpr BasicString& push_back(char i, u64 repeat = 1) noexcept
    {
      for (size_t z = 0; z < repeat; z++)
        BasicString::UnderlyingVector::push_back(i);
      return *this;
    }

    /// @brief Gets a line from a file.
    /// The resulting BasicString is not NUL terminated, and does not contain the new line.
    /// FILE_EOF and FILE_ERROR is only returned if no characters were read.
    /// @param from The (opened) file from which to read characters
    /// @param reserve The count of characters to reserve before reading characters
    /// @param strip_front If true, skips all blank (' ', '\\t') characters in the front of the string
    /// @return BasicString containing the line or either FILE_EOF or FILE_ERROR.
    static Expect<BasicString, io::IOError> getLine(FILE* from, u64 reserve = 64, bool strip_front = true) noexcept
    {
      BasicString str;
      auto gchar = std::fgetc(from);
      if (gchar == EOF)
      {
        if (feof(from))
          return { Error, io::IOError::FILE_EOF };
        else
          return { Error, io::IOError::FILE_ERROR };
      }
      if (static_cast<unsigned char>(gchar) > 127)
        return { Error, io::IOError::INVALID_ENCODING };

      str.reserve(reserve);
      if (strip_front && std::isblank(gchar))
      {
        //Consume spaces
        while ((gchar = std::fgetc(from)) != EOF)
        {
          if (!clt::isblank(gchar))
            break;
          else if (static_cast<unsigned char>(gchar) > 127)
            return { Error, io::IOError::INVALID_ENCODING };
        }
      }
      for (;;)
      {
        if (gchar != '\n' && gchar != EOF)
        {
          if (static_cast<unsigned char>(gchar) > 127)
            return { Error, io::IOError::INVALID_ENCODING };
          str.push_back(static_cast<char>(gchar));
          gchar = std::fgetc(from);
        }
        else
          break;
      }
      return str;
    }

    /// @brief Gets a line from a 'stdin'.
    /// The resulting BasicString is not NUL terminated, and does not contain the new line.
    /// FILE_EOF and FILE_ERROR is only returned if no characters were read.
    /// @param reserve The count of characters to reserve before reading characters
    /// @param strip_front If true, skips all blank (' ', '\\t') characters in the front of the string
    /// @return BasicString containing the line or one of [FILE_EOF, FILE_ERROR, INVALID_ENCODING].
    static Expect<BasicString, io::IOError> getLine(u64 reserve = 64, bool strip_front = true) noexcept
    {
      return getLine(stdin, reserve, strip_front);
    }

    /// @brief Returns the content of a file
    /// @param name The path of the file
    /// @return BasicString containing the line or one of [FILE_EOF, FILE_ERROR, INVALID_ENCODING].
    static Expect<BasicString, io::IOError> getFile(const char* name) noexcept
    {
      BasicString str;

      FILE* file = fopen(name, "rb");

      ON_SCOPE_EXIT
      {
        if (file != nullptr)
          fclose(file);
      };

      if (file == nullptr)
        return { Error, io::IOError::FILE_ERROR };
      if (fseek(file, 0L, SEEK_END) != 0)
        return { Error, io::IOError::FILE_ERROR };
      auto sz = ftell(file);
      if (sz == -1)
        return { Error, io::IOError::FILE_ERROR };
      rewind(file);

      str.reserve(sz);
      if (fread(str.data(), sizeof(char), sz, file) != sz)
        return { Error, io::IOError::FILE_ERROR };

      for (auto i : str)
      {
        if (static_cast<unsigned char>(i) > 127)
          return { Error, io::IOError::INVALID_ENCODING };
      }
      str._Unsafe_size(sz);
      return str;
    }

    /// @brief Check if every object of v1 and v2 are equal
    /// @param v1 The first strings
    /// @param v2 The second strings
    /// @return True if both strings are equal
    friend constexpr bool operator==(const BasicString& v1, const StringView& v2) noexcept
    {
      if (v1.size() != v2.size())
        return false;
      for (size_t i = 0; i < v1.size(); i++)
        if (v1[i] != v2[i])
          return false;
      return true;
    }

    /// @brief Lexicographically compare two strings
    /// @param v1 The first strings
    /// @param v2 The second strings
    /// @return Result of comparison
    friend constexpr auto operator<=>(const BasicString& v1, StringView v2) noexcept
    {
      return std::lexicographical_compare_three_way(
        v1.begin(), v1.end(), v2.begin(), v2.end()
      );
    }
  };

  /// @brief ASCII String
  using String = BasicString<mem::GlobalAllocatorDescription, StringEncoding::ASCII>;

  template<auto ALLOCATOR>
  /// @brief clt::hash overload for String
  struct hash<BasicString<ALLOCATOR, StringEncoding::ASCII>>
  {
    /// @brief Hashing operator
    /// @param value The value to hash
    /// @return Hash
    constexpr size_t operator()(const BasicString<ALLOCATOR, StringEncoding::ASCII>& value) const noexcept
    {
      return hash_value<StringView>(value);
    }
  };

  template<>
  /// @brief clt::hash overload for StringView
  struct hash<StringView>
  {
    /// @brief Hashing operator
    /// @param value The value to hash
    /// @return Hash
    constexpr size_t operator()(const StringView& value) const noexcept
    {
      uint64_t hash = 0xCBF29CE484222325;
      for (auto i : value)
      {
        hash ^= (uint8_t)i;
        hash *= 0x100000001B3; //FNV prime
      }
      return hash;
    }
  };
}

template<>
struct scn::scanner<clt::String>
  : scn::scanner<clt::StringView>
{
  template <typename Context>
  error scan(clt::String& val, Context& ctx)
  {
    clt::StringView strv;
    auto r = scn::scanner<clt::StringView>::scan(strv, ctx);
    if (r)
      val = strv;
    return r;
  }
};

template<>
struct fmt::formatter<clt::String>
{
  template<typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    auto it = ctx.begin();
    auto end = ctx.end();
    assert_true("Possible format for String is: {}!", it == end);
    return it;
  }

  template<typename FormatContext>
  auto format(const clt::String& str, FormatContext& ctx)
  {
    return fmt::format_to(ctx.out(), "{:.{}}", str.data(), str.size());
  }
};

#endif //!HG_COLT_STRING