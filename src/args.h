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

    cl::Opt<"o", cl::desc<"Output file name">,
      cl::location<OutputFile>>,

    cl::OptPos<"input_file", cl::desc<"The input file">,
      cl::location<InputFile>>
    > ;
}


#endif //!HG_CMD_OPTIONS