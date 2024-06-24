#ifndef HG_COLTI_EXE
#define HG_COLTI_EXE

#include "common/colt_pch.h"

namespace clt::run
{
  /// @brief Represents a section of a Colti executable
  struct ColtiSection
  {
    /// @brief The name of the section
    StringView name;
    /// @brief The beginning of the section
    const u8* begin;
    /// @brief The end of the section
    const u8* end;
  };

  /// @brief Represents the starting header of a Colti executable.
  /// All numbers are in little endian.
  struct ColtiHeader
  {
    /// @brief This magic number is COLT in ASCII
    static constexpr u32 MAGIC_NUMBER = htol(static_cast<u32>(0x434F4C54));

    /// @brief The number of section in the current executable
    u16 section_count;

    // GROUP OF 2 BYTES vvv

    /// @brief The major version of the language
    u16 colt_version_major : 5;
    /// @brief The minor version of the language
    u16 colt_version_minor : 5;
    /// @brief The tweak version of the language
    u16 colt_version_tweak : 6;

    // GROUP OF 2 BYTES vvv

    /// @brief True if the 'compilation_date_hour' (below) is AM, false if PM
    u16 compilation_date_am : 1;
    /// @brief The compilation day (0 means no date information, 1 -> Monday...)
    u16 compilation_date_day : 3;
    /// @brief The compilation month (0 means no date information, 1 -> January...)
    u16 compilation_date_month : 4;
    /// @brief The compilation year (0 means no date information, 1 -> 2024...)
    u16 compilation_date_year : 8;

    // GROUP OF 2 BYTES vvv

    /// @brief The compilation second (0 means no date information, 1 -> 0s...)
    u16 compilation_date_sec : 6;
    /// @brief The compilation minute (0 means no date information, 1 -> 0m...)
    u16 compilation_date_min : 6;
    /// @brief The compilation hour (0 means no date information, 1 -> 0h).
    /// If 'compilation_date_am' is false then we add 12 to the encoded hour.
    u16 compilation_date_hour : 4;
    
    /// @brief This must be equal to 'MAGIC_NUMBER'
    u32 magic_number;
  };

  class ColtiExecutable
  {
    //ColtiExecutable()
  public:

  };

} // namespace clt::run

#endif // !HG_COLTI_EXE
