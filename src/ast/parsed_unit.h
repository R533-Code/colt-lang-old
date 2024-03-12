/*****************************************************************//**
 * @file   parsed_unit.h
 * @brief  Contains ParsedUnit, the result of parsing a single file.
 * A ParsedUnit needs to have access to the global table, which
 * is shared by all ParsedUnits.
 * 
 * @author RPC
 * @date   March 2024
 *********************************************************************/
#ifndef HG_COLT_PARSED_UNIT
#define HG_COLT_PARSED_UNIT

namespace clt::lng
{
  // Forward declaration
  class ParsedProgram;

  class ParsedUnit
  {
    /// @brief Contains the global table, and the error reporter
    ParsedProgram& program;
  };
}

#endif // !HG_COLT_PARSED_UNIT
