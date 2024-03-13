/*****************************************************************//**
 * @file   parsed_program.h
 * @brief  Contains ParsedProgram, the result of the compiler's front-end.
 * 
 * @author RPC
 * @date   March 2024
 *********************************************************************/
#ifndef HG_COLT_PARSED_PROGRAM
#define HG_COLT_PARSED_PROGRAM

#include "util/colt_pch.h"
#include "lng/colt_type_buffer.h"
#include "lng/colt_module.h"
#include "err/composable_reporter.h"
#include "parsed_unit.h"
#include "structs/map.h"

namespace clt::lng
{
  /// @brief Represents the ASTs of all files.
  /// This is the result of the compiler's front-end, that can
  /// be sent to any backend for lowering into useful code.
  class ParsedProgram
  {
    /// @brief Contains all the types of the program
    TypeBuffer type_buffer{};
    /// @brief Contains all the modules of the program
    ModuleBuffer module_buffer{};
    /// @brief Contains all the parsed units
    Map<std::filesystem::path, ParsedUnit> parsed_units{};
    /// @brief The reporter used to generate warnings and errors
    ErrorReporter& reporter;
    /// @brief The starting file to parse
    const std::filesystem::path& start_file;
    /// @brief Represents the include paths of the program
    const Vector<std::filesystem::path>& includes;

  public:
    /// @brief Constructs a parsed program.
    /// This does not parse anything.
    /// @param reporter The reporter used for errors and warnings
    /// @param includes The include path used by the program
    ParsedProgram(ErrorReporter& reporter, const std::filesystem::path& start, const Vector<std::filesystem::path>& includes) noexcept
      : reporter(reporter), start_file(start), includes(includes) {}

    /// @brief Returns the reporter used for errors and warnings
    /// @return The reporter
    ErrorReporter& getReporter() noexcept { return reporter; }

    /// @brief Adds an import
    /// @return True if the import was successful, false on failure
    bool importUnit(StringView import_path) noexcept;
  };
}

#endif // !HG_COLT_PARSED_PROGRAM
