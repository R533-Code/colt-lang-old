/*****************************************************************/ /**
 * @file   parsed_program.cpp
 * @brief  Contains the implementation of ParsedProgram
 *
 * @author RPC
 * @date   March 2024
 *********************************************************************/
#include "parsed_program.h"
#include "parsed_unit.h"
#include "ast.h"

namespace clt::lng
{
  std::filesystem::path ParsedProgram::EMPTY_PATH = "";

  ParsedProgram::ParsedProgram(
      ErrorReporter& reporter, const std::filesystem::path& start,
      const Vector<std::filesystem::path>& includes,
      const WarnFor& warn_for) noexcept
      : _reporter(reporter)
      , start_file(start)
      , includes(includes)
      , _warn_for(warn_for)
  {
    parsed_units.insert(EMPTY_PATH, ParsedUnit{*this, start}).first->second.parse();
  }

  ParsedProgram::ParsedProgram(
      ErrorReporter& reporter, StringView start,
      const Vector<std::filesystem::path>& includes,
      const WarnFor& warn_for) noexcept
      : _reporter(reporter)
      , start_file(EMPTY_PATH)
      , includes(includes)
      , _warn_for(warn_for)
  {
    parsed_units.insert(EMPTY_PATH, ParsedUnit{*this, start}).first->second.parse();
  }

  bool ParsedProgram::import_unit(StringView import_path) noexcept
  {
    return false;
  }
} // namespace clt::lng