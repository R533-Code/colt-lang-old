#ifndef HG_COLTI_EXE
#define HG_COLTI_EXE

#include "common/colt_pch.h"

namespace clt::run
{
  using time_point = std::chrono::time_point<std::chrono::system_clock>;

  /// @brief Represents the starting header of a Colti executable.
  /// All numbers are in little endian.
  class ColtiHeader
  {
  public:
    /// @brief This magic number is TLOC (for COLT) in ASCII
    static constexpr u32 MAGIC_NUMBER = htol(static_cast<u32>(0x434F4C54));

  private:
    // While bit fields could have simplified the code,
    // they don't play well with endianness.

    /// @brief The number of section in the current executable
    u16 section_count = 0;

    /// @brief The version of the language [5b MAJOR][5b MINOR][6b PATCH]
    u16 colt_version = 0;

    /// @brief The compilation date hour and month [4b HOUR][4b MONTH].
    /// The hour is represented as 0 means no date information, 1 -> 0h...
    /// The month is represented as 0 means no date information, 1 -> January...
    u8 compilation_date_hour_month = 0;
    /// @brief The compilation minute and bool for AM/PM [0][6b MINUTE][1b AM].
    /// The minute is represented as 0 means no date information, 1 -> 0m...
    /// If AM is false then we add 12 to the encoded hour.
    u8 compilation_date_minute_am = 0;
    /// @brief The compilation year and day [11b YEAR][5b DAY]
    /// The day is represented as 0 means no date information, [1-31]...
    /// The year is represented as 0 means no date information, 1 -> 2024...
    u16 compilation_date_year_day = 0;

    /// @brief This must be equal to 'MAGIC_NUMBER'
    u32 magic_number = MAGIC_NUMBER;
    /// @brief Unused for now
    u32 padding = 0;

    // After the padding, there is a u64 containing the offset
    // to each section.
    // Each section begins with the NUL-terminated section name
    // (31 characters at most, without counting NUL-terminator),
    // followed by a u64 representing the size in bytes of the
    // content, then followed by the content of the section.
    // The content of the section must always be 8-byte aligned.
    // u64 offset_to_section[section_count]

    /// @brief Encodes a version
    /// @param version The version to encode
    /// @return The encoded version
    static constexpr u16 encode_version(const ColtVersion& version) noexcept;
    /// @brief Decodes a version
    /// @param version The version to decode
    /// @return The decoded version
    static constexpr ColtVersion decode_version(u16 version) noexcept;

    /// @brief Encodes a time point
    /// @param tm The time point
    /// @param compilation_date_hour_month The compilation hour and month
    /// @param compilation_date_minute_am The compilation minute and AM/PM flag
    /// @param compilation_date_year_day The compilation year and day
    static constexpr void encode_time(
        time_point tm, u8& compilation_date_hour_month,
        u8& compilation_date_minute_am, u16& compilation_date_year_day) noexcept;

    /// @brief Decodes a time point
    /// @param compilation_date_hour_month The compilation hour and month
    /// @param compilation_date_minute_am The compilation minute and AM/PM flag
    /// @param compilation_date_year_day The compilation year and day
    /// @return The decoded time point or null
    static constexpr Option<time_point> decode_time(
        u8 compilation_date_hour_month, u8 compilation_date_minute_am,
        u16 compilation_date_year_day) noexcept;

  public:
    /// @brief Initializes to empty header (no sections).
    /// The header is still valid, but not useful.
    constexpr ColtiHeader() noexcept = default;

    /// @brief Constructor
    /// @param section_count The section count
    /// @param version The language version
    /// @param time_point The compilation time stamp or None
    constexpr ColtiHeader(
        u16 section_count, const ColtVersion& version,
        Option<time_point> time_point);

    /// @brief Returns the compilation time or None if it doesn't exist.
    /// This function decodes the value every time it is called, so
    /// for performance cache result!
    /// @return The compilation time or None
    constexpr Option<time_point> compilation_time() const noexcept
    {
      return decode_time(
          compilation_date_hour_month, compilation_date_minute_am,
          compilation_date_year_day);
    }

    /// @brief Returns the language version
    /// @return The language version
    constexpr ColtVersion version() const noexcept
    {
      return decode_version(colt_version);
    }

    /// @brief Returns the number of sections
    /// @return The number of sections
    constexpr u16 sections() const noexcept
    {
      return section_count;
    }

    /// @brief The magic number to verify that the header is a Colt header
    /// @return Magic number (must be MAGIC_NUMBER to be valid)
    constexpr u32 signature() const noexcept
    {
      return magic_number;
    }
  };

  /// @brief Aligns a value to the next 'ALIGN' boundary
  /// @tparam T The type of the value to align
  /// @tparam ALIGN The alignment (must be a multiple of 2!)
  /// @param to_align The value to align
  /// @return The value aligned to the next closest boundary
  template<u8 ALIGN, typename T>
    requires std::is_pointer_v<T> || meta::UnsignedIntegral<T>
  constexpr T align_to_next(T to_align) noexcept;

  /// @brief Represents an executable section
  struct ExecutableSection
  {
    /// @brief The name of the section
    StringView name;
    /// @brief The beginning of the section
    const u8* begin;
    /// @brief The size of the section
    u64 size;
  };

  class ColtiExecutable
  {
    View<u8> bytes;

    ColtiExecutable(View<u8> bytes) noexcept
        : bytes(bytes)
    {
    }

  public:
    static Option<ColtiExecutable> load(View<u8> bytes) noexcept;

    /// @brief Returns the ColtiHeader of the executable.
    /// It is error prone to use the header directly: every information
    /// it provides can be accessed safely using the other member functions.
    /// @return The ColtiHeader of the executable
    const ColtiHeader* header() const noexcept
    {
      return reinterpret_cast<const ColtiHeader*>(bytes.data());
    }

    /// @brief Returns the compilation time or None if it doesn't exist.
    /// @return The compilation time or None
    Option<time_point> compilation_time() const noexcept;

    /// @brief Returns the language version of the compiled executable
    /// @return The language version of the compiled executable
    ColtVersion version() const noexcept;

    /// @brief Returns the number of sections contained in the executable
    /// @return The section count
    u16 section_count() const noexcept;

    /// @brief Returns the section at index 'index'
    /// @param index The index of the section (index < section_count)
    /// @return Section at index 'index'
    ExecutableSection section(u16 index) const noexcept;

    /// @brief Returns the section name at index 'index'
    /// @param index The index of the section (index < section_count)
    /// @return The section name
    StringView section_name(u16 index) const noexcept;

    /// @brief Searches for a section of name 'name'
    /// @param name The name of the section
    /// @return The section or None if not found
    Option<ExecutableSection> find_section(StringView name) const noexcept;

    /// @brief Returns a view over the sections offset
    /// @return View over the sections offset
    View<u64> section_offsets() const noexcept;

    /// @brief Check if an offset points inside the executable
    /// @param offset The offset (from the start of the executable)
    /// @return True if the offset points inside the executable
    bool is_in_range(u64 offset) const noexcept { return offset < bytes.size(); }
  };

  template<u8 ALIGN, typename T>
    requires std::is_pointer_v<T> || meta::UnsignedIntegral<T>
  constexpr T align_to_next(T to_align) noexcept
  {
    const auto VALUE = reinterpret_cast<uintptr_t>(to_align) % ALIGN;
    return to_align + (ALIGN - VALUE) * (VALUE != 0);
  }

  constexpr u16 ColtiHeader::encode_version(const ColtVersion& version) noexcept
  {
    u16 version_encoded = 0;
    version_encoded |= version.major << 11;
    version_encoded |= (version.minor & 0b1'11'11) << 6;
    version_encoded |= (version.patch & 0b11'11'11);
    return htol(version_encoded);
  }

  constexpr ColtVersion ColtiHeader::decode_version(u16 version) noexcept
  {
    ColtVersion to_ret;
    version      = ltoh(version);
    to_ret.major = version >> 11;
    to_ret.minor = (version >> 6) & 0b1'11'11;
    to_ret.patch = (version & 0b11'11'11);
    return to_ret;
  }

  constexpr void ColtiHeader::encode_time(
      time_point tm, u8& compilation_date_hour_month, u8& compilation_date_minute_am,
      u16& compilation_date_year_day) noexcept
  {
    using namespace date;

    auto dp = floor<days>(tm);
    year_month_day ymd{dp};
    hh_mm_ss time{floor<std::chrono::minutes>(tm - dp)};
    auto y = ymd.year();
    auto m = ymd.month();
    auto d = ymd.day();
    auto h = time.hours();
    auto M = time.minutes();

    assert_true("Invalid year!", (i32)y >= 2024);
    compilation_date_hour_month = static_cast<u8>(h.count() + 1) << 4;
    compilation_date_hour_month |= static_cast<u8>((u32)m + 1) & 0b1111;

    compilation_date_minute_am = static_cast<u8>(M.count() + 1) << 1;
    compilation_date_minute_am |= static_cast<u8>(h.count() < 12);

    compilation_date_year_day =
        (static_cast<u16>((i32)y - 2023) & 0b1'11'11'11'11'11) << 5;
    compilation_date_year_day |= static_cast<u16>((u32)d + 1);
    // Make it little endian
    compilation_date_year_day = htol(compilation_date_year_day);
  }

  constexpr Option<time_point> ColtiHeader::decode_time(
      u8 compilation_date_hour_month, u8 compilation_date_minute_am,
      u16 compilation_date_year_day) noexcept
  {
    namespace tm = date;
    
    compilation_date_year_day = ltoh(compilation_date_year_day);

    auto hour   = static_cast<u8>(compilation_date_hour_month >> 4);
    auto month  = static_cast<u8>(compilation_date_hour_month & 0b1111);
    auto minute = static_cast<u8>(compilation_date_minute_am >> 1);
    auto is_am  = !static_cast<bool>(compilation_date_minute_am & 0b1);
    auto year   = static_cast<u16>(compilation_date_year_day >> 5);
    auto day    = static_cast<u8>(compilation_date_year_day & 0b1'11'11);

    // If any information is missing discard all the other
    if (hour == 0 || month == 0 || minute == 0 || year == 0 || day == 0)
      return None;
    hour--;
    hour += ((u8)is_am * 12);
    month--;
    minute--;
    year += 2023;
    day--;
    
    // Reconstruct the time point
    return time_point(tm::sys_days{tm::year{year} / tm::month{month} / tm::day{day}})
           + std::chrono::hours{hour} + std::chrono::minutes{minute};
  }

  constexpr ColtiHeader::ColtiHeader(
      u16 section_count, const ColtVersion& version, Option<time_point> time_point)
      : section_count(section_count)
      , colt_version(encode_version(version))
  {
    // Encode the time stamp
    if (time_point.is_value())
    {
      encode_time(
          time_point.value(), compilation_date_hour_month,
          compilation_date_minute_am, compilation_date_year_day);
    }
  }

} // namespace clt::run

#endif // !HG_COLTI_EXE