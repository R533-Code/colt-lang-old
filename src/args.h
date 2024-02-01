/*****************************************************************//**
 * @file   args.h
 * @brief  Contains the predefined command line arguments.
 * 
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_CMD_OPTIONS
#define HG_CMD_OPTIONS

#include <io/print.h>
#include "util/colt_config.h"
#include <util/args_parsing.h>

namespace clt
{
  /// @brief Flag to check if the application must wait for input before exiting.
  inline bool WaitForUserInput = true;
  /// @brief Number of spaces to use when transpiling code
  inline u8 OutputSpace = 2;
  /// @brief The output file name
  inline std::string_view OutputFile = {};
  /// @brief The input file name
  inline std::string_view InputFile = {};
  /// @brief The maximum number of messages
  inline Option<u16> MaxMessages = 128;
  /// @brief The maximum number of warnings
  inline Option<u16> MaxWarnings = 64;
  /// @brief The maximum number of errors
  inline Option<u16> MaxErrors = 32;

  namespace details
  {
    /// @brief Callback to validate arguments of Max* globals
    /// @param to_validate The global to validate, as an example MaxErrors
    /// @param flag The flag name, as an example "-max-error"
    /// @param init The value to assign on invalid input
    constexpr void max_reporter_validator(Option<u16>& to_validate, const char* flag, const Option<u16>& init) noexcept
    {
      if (to_validate.is_none() || (to_validate.value() != 0))
        return;
      io::print_warn("'{}' is not a valid value for flag '{}'!", to_validate, flag);
      to_validate = init;
    }
  }

  /// @brief The meta type used to generated command line argument handling function
  using CMDs = meta::type_list<
    cl::Opt<"nocolor", cl::desc<"Turns off colored output">, cl::alias<"C">,
      cl::callback<[]{ clt::io::OutputColor = false; }>>,

    cl::Opt<"nowait", cl::desc<"Do not wait for user input">,
      cl::callback<[]{ clt::WaitForUserInput = false; }>>,

    cl::Opt<"v", cl::desc<"Prints the version of the compiler">,
      cl::callback<[]{ io::print("COLT v{} on {} ({}).", COLT_VERSION_STRING, COLT_OS_STRING, COLT_CONFIG_STRING); std::exit(0); }>>,

    cl::Opt<"space", cl::desc<"Chooses the number of spaces when transpiling">,
      cl::value_desc<"[0-255]">, cl::location<OutputSpace>>,
    
    cl::Opt<"max-error", cl::desc<"Chooses the maximum number of errors reported">,
      cl::value_desc<"[None|1-65536]">, cl::location<MaxErrors>,
      cl::callback<[] { details::max_reporter_validator(MaxErrors, "-max-error", 32); } >>,
    
    cl::Opt<"max-warn", cl::desc<"Chooses the maximum number of warnings reported">,
      cl::value_desc<"[None|1-65536]">, cl::location<MaxWarnings>,
      cl::callback<[] { details::max_reporter_validator(MaxWarnings, "-max-warn", 64); } >>,

    cl::Opt<"max-msg", cl::desc<"Chooses the maximum number of messages reported">,
      cl::value_desc<"[None|1-65536]">, cl::location<MaxMessages>,
      cl::callback<[] { details::max_reporter_validator(MaxMessages, "-max-msg", 128); } >>,

    cl::Opt<"o", cl::desc<"Output file name">,
      cl::location<OutputFile>>,

    cl::OptPos<"input_file", cl::desc<"The input file">,
      cl::location<InputFile>>
    > ;
}


#endif //!HG_CMD_OPTIONS