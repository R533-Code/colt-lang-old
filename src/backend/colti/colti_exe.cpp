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
    if (reinterpret_cast<const ColtiHeader*>(bytes.data())->signature()
        != ColtiHeader::MAGIC_NUMBER)
      return None;
    return ColtiExecutable(bytes);
  }

  Option<time_point> ColtiExecutable::compilation_time() const noexcept
  {
    return header()->compilation_time();
  }

  ColtVersion ColtiExecutable::version() const noexcept
  {
    return header()->version();
  }

  u16 ColtiExecutable::section_count() const noexcept
  {
    return header()->sections();
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