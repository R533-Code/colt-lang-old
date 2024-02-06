/*****************************************************************//**
 * @file   main.cpp
 * @brief  Starting point of the compiler.
 * 
 * @author RPC
 * @date   February 2024
 *********************************************************************/
#include <util/colt_pch.h>
#include "args.h"
#include "lex/colt_lexer.h"

using namespace clt;

int main(int argc, const char** argv)
{
  mem::global_on_null([]() noexcept { io::print_fatal("Could not allocate memory!"); });
  cl::parse_command_line_options<CMDs>(argc, argv);
  
  if (InputFile.empty())
    io::print_warn("REPL is not implemented...");
  else
    io::print_warn("Transpilation is not implemented...");

  if (WaitForUserInput)
    io::press_to_continue();
}