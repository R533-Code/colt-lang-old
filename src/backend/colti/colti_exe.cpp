#include "colti_exe.h"

namespace clt::run
{
  Option<ColtiExecutable> ColtiExecutable::load(View<u8> bytes) noexcept
  {
    assert_true(
        "Bytes must be aligned!",
        (uintptr_t)bytes.data() % alignof(ColtiHeader) == 0);

    if (bytes.size() < sizeof(ColtiHeader))
      return None;
    // We now need to check that this is a valid header
    if (reinterpret_cast<const ColtiHeader*>(bytes.data())->magic_number
        != ColtiHeader::MAGIC_NUMBER)
      return None;
    return ColtiExecutable(bytes);
  }

  Option<std::chrono::years> ColtiExecutable::year() const noexcept
  {
    const i32 value = (u8)header()->compilation_date_year;
    if (value == 0)
      return None;
    return std::chrono::years(value + 2024);
  }

  Option<std::chrono::months> ColtiExecutable::month() const noexcept
  {
    const u8 value = (u8)header()->compilation_date_month;
    if (value == 0 || value > 12)
      return None;
    return std::chrono::months(value);
  }

  Option<std::chrono::days> ColtiExecutable::day() const noexcept
  {
    const u8 value = (u8)header()->compilation_date_day;
    if (value == 0)
      return None;
    return std::chrono::days(value);
  }

  Option<std::chrono::minutes> ColtiExecutable::minute() const noexcept
  {
    // compilation_date_min == 1 -> hour == 0
    // compilation_date_min == 60 -> hour == 59
    u32 min = (u32)header()->compilation_date_min;
    if (min == 0 || min > 60)
      return None;
    return std::chrono::minutes{min};
  }

  Option<std::chrono::hours> ColtiExecutable::hour() const noexcept
  {
    u32 hour = (u32)header()->compilation_date_hour;
    // compilation_date_hour == 1 -> hour == 0
    // compilation_date_hour == 12 -> hour == 11
    if (hour == 0 || hour > 12)
      return None;
    hour--;
    hour += 12 * (u8)(!header()->compilation_date_am);
    return std::chrono::hours{hour};
  }

  ColtVersion ColtiExecutable::version() const noexcept
  {
    return {
        (u8)header()->colt_version_major, (u8)header()->colt_version_minor,
        (u8)header()->colt_version_patch};
  }

  u16 ColtiExecutable::section_count() const noexcept
  {
    return header()->section_count;
  }

  ExecutableSection ColtiExecutable::section(u16 index) const noexcept
  {
    assert_true("Invalid index!", index < section_count());

    const u8* base_offset = bytes.data();
    auto section_name =
        reinterpret_cast<const char*>(base_offset + section_offsets()[index]);

    // The size of the 'section_name' string (which can be
    // at maximum 31) without the NUL-terminator
    u64 section_name_size = 0;
    for (size_t i = 0; i < 32; i++)
    {
      if (section_name[i] == '\0')
        break;
      section_name_size++;
    }
    // Align pointer to the next u64
    auto section_size_ptr = reinterpret_cast<const u64*>(
        align_to_next<8>(section_name + section_name_size + 1));

    return ExecutableSection{
        StringView{section_name, section_name_size},
        reinterpret_cast<const u8*>(section_size_ptr + 1), *section_size_ptr};
  }

  StringView ColtiExecutable::section_name(u16 index) const noexcept
  {
    return section(index).name;
  }

  Option<ExecutableSection> ColtiExecutable::find_section(
      StringView name) const noexcept
  {
    for (size_t i = 0; i < section_count(); i++)
      if (auto section_ = section(i); section_.name == name)
        return section_;
    return None;
  }

  View<u64> ColtiExecutable::section_offsets() const noexcept
  {
    return {
        reinterpret_cast<const u64*>(bytes.data() + sizeof(ColtiHeader)),
        section_count()};
  }
} // namespace clt::run