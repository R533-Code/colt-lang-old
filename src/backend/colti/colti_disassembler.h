#ifndef HG_COLTI_DISASSEMBLER
#define HG_COLTI_DISASSEMBLER

#include "colti_exe.h"

namespace clt
{
  /// @brief Disassembles a file, printing the result to stdout
  /// @param file The file to disassemble
  void disassemble_file(StringView file) noexcept;
} // namespace clt

#endif // !HG_COLTI_DISASSEMBLER
