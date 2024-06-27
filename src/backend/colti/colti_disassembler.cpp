#include "colti_disassembler.h"

namespace clt
{
  void disassemble_file(StringView file) noexcept
  {
    using namespace run;

    auto str = String::getFile(file.data());
    if (str.is_error())
      return io::print_error("Could not open file at path '{}'!", file);

    auto exe_o = ColtiExecutable::load(
        {reinterpret_cast<const u8*>(str->data()), str->size()});

    if (exe_o.is_none())
    {
      return io::print_error(
          "File at path '{}' is not a valid colti executable!", file);
    }

    auto& exe = *exe_o;
    io::print("Disassembly of '{}': ", file);
    if (auto date = exe.compilation_time(); date.is_value())
    {
      io::print(
          "Compiled on {} with Colt version '{}.{}.{}'.", *date, exe.version().major,
          exe.version().minor, exe.version().patch);
    }
    else
    {
      io::print(
          "Compiled with Colt version '{}.{}.{}'.", exe.version().major,
          exe.version().minor, exe.version().patch);
    }

    io::print(
        "Executable has {} section{}", exe.section_count(),
        exe.section_count() == 1 ? "." : "s.");
    for (u16 i = 0; i < exe.section_count(); i++)
    {
      auto section = exe.section(i);
      io::print(
          "  - {}: {} ({} byte{})", i, section.name, section.size,
          section.size == 1 ? "." : "s.");
    }
  }
}

