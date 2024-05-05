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
#include "err/warn.h"

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
    /// @brief The set of all literal strings in the program
    StableSet<String> literal_str{};
    /// @brief The reporter used to generate warnings and errors
    ErrorReporter& reporter;
    /// @brief The starting file to parse
    const std::filesystem::path& start_file;
    /// @brief Represents the include paths of the program
    const Vector<std::filesystem::path>& includes;
    /// @brief Dictates which warnings to generate
    WarnFor warn_for;

  public:
    /// @brief Represents an empty path (used when the StringView constructor overload is used)
    static std::filesystem::path EMPTY_PATH;
    
    /// @brief Constructs a parsed program.
    /// This does not parse anything.
    /// @param reporter The reporter used for errors and warnings
    /// @param start The starting file to parse (main.ct)
    /// @param includes The include path used by the program
    /// @param warn_for The warnings to reports
    explicit ParsedProgram(ErrorReporter& reporter, const std::filesystem::path& start, const Vector<std::filesystem::path>& includes, const WarnFor& warn_for) noexcept;

    /// @brief Constructs a parsed program.
    /// This does not parse anything.
    /// @param reporter The reporter used for errors and warnings
    /// @param start The starting string to parse (for REPL)
    /// @param includes The include path used by the program
    /// @param warn_for The warnings to reports
    explicit ParsedProgram(ErrorReporter& reporter, StringView start, const Vector<std::filesystem::path>& includes, const WarnFor& warn_for) noexcept;

    /// @brief Returns the reporter used for errors and warnings
    /// @return The reporter
    ErrorReporter& getReporter() noexcept { return reporter; }
    /// @brief Returns the reporter used for errors and warnings
    /// @return The reporter
    const ErrorReporter& getReporter() const noexcept { return reporter; }
    
    /// @brief Returns the type buffer used for errors and warnings
    /// @return The type buffer
    TypeBuffer& getTypes() noexcept { return type_buffer; }
    /// @brief Returns the type buffer used for errors and warnings
    /// @return The type buffer
    const TypeBuffer& getTypes() const noexcept { return type_buffer; }

    /// @brief Returns the set of literal strings in the program
    /// @return Set of literal strings in the program
    StableSet<String>& getStrLiterals() noexcept { return literal_str; }    
    /// @brief Returns the set of literal strings in the program
    /// @return Set of literal strings in the program
    const StableSet<String>& getStrLiterals() const noexcept { return literal_str; }

    /// @brief Returns what to warn for
    /// @return What to warn for
    WarnFor& getWarnFor() noexcept { return warn_for; }
    /// @brief Returns what to warn for
    /// @return What to warn for
    const WarnFor& getWarnFor() const noexcept { return warn_for; }

    /// @brief Adds an import
    /// @return True if the import was successful, false on failure
    bool importUnit(StringView import_path) noexcept;
  };
}

#endif // !HG_COLT_PARSED_PROGRAM
