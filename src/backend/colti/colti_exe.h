#ifndef HG_COLTI_EXE
#define HG_COLTI_EXE

#include "common/colt_pch.h"

namespace clt::run
{
  /// @brief Represents the starting header of a Colti executable.
  /// All numbers are in little endian.
  struct ColtiHeader
  {
    /// @brief This magic number is COLT in ASCII
    static constexpr u32 MAGIC_NUMBER = htol(static_cast<u32>(0x434F4C54));

    /// @brief The number of section in the current executable
    u16 section_count;

    /// @brief The major version of the language
    u16 colt_version_major : 5;
    /// @brief The minor version of the language
    u16 colt_version_minor : 5;
    /// @brief The patch version of the language
    u16 colt_version_patch : 6;

    /// @brief The compilation hour (0 means no date information, 1 -> 0h).
    /// If 'compilation_date_am' is false then we add 12 to the encoded hour.
    u16 compilation_date_hour : 4;
    /// @brief The compilation month (0 means no date information, 1 -> January...)
    u16 compilation_date_month : 4;
    /// @brief True if the 'compilation_date_hour' is AM, false if PM
    u16 compilation_date_am : 1;
    /// @brief The compilation minute (0 means no date information, 1 -> 0m...)
    u16 compilation_date_min : 6;
    /// @brief The compilation day (0 means no date information, [1-31]...)
    u16 compilation_date_day : 5;
    /// @brief The compilation year (0 means no date information, 1 -> 2024...)
    u16 compilation_date_year : 11;

    /// @brief This must be equal to 'MAGIC_NUMBER'
    u32 magic_number;
    /// @brief Unused for now
    u32 padding;
    
    // After the padding, there is a u64 containing the offset
    // to each section.
    // Each section begins with the NUL-terminated section name
    // (31 characters at most, without counting NUL-terminator),
    // followed by a u64 representing the size in bytes of the
    // content, then followed by the content of the section.
    // The content of the section must always be 8-byte aligned.
    // u64 offset_to_section[section_count]
  };
  
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

    /// @brief Returns the compilation year of the executable
    /// @return Year or None if the info is unavailable
    Option<std::chrono::year> year() const noexcept;
    /// @brief Returns the compilation month of the executable
    /// @return Month that is always ok() or None if the info is unavailable
    Option<std::chrono::month> month() const noexcept;
    /// @brief Returns the compilation year of the executable
    /// @return Year or None if the info is unavailable
    Option<std::chrono::day> day() const noexcept;
    /// @brief Returns the compilation minute of the executable.
    /// If a value is return, it will be in range [0, 59].
    /// @return Minute or None if the info is unavailable
    Option<std::chrono::minutes> minute() const noexcept;
    /// @brief Returns the compilation hour of the executable.
    /// If a value is return, it will be in range [0, 23].
    /// @return Hour or None if the info is unavailable
    Option<std::chrono::hours> hour() const noexcept;

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

} // namespace clt::run

#endif // !HG_COLTI_EXE
