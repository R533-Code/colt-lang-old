/*****************************************************************//**
 * @file   main.cpp
 * @brief  Starting point of the compiler.
 * 
 * @author RPC
 * @date   February 2024
 *********************************************************************/
#include <util/colt_pch.h>
#include "args.h"
#include "util/run_tests.h"
#include "type/colt_type_buffer.h"

using namespace clt;

void REPL()
{
  io::print_warn("REPL is not implemented...");

  auto reporter = lng::make_error_reporter<lng::ConsoleReporter>();
  while (true)
  {
    io::print<"">("Enter line to lex: ");
    auto a = String::getLine(64, false);
    if (a.is_error())
      return;
    const auto& str = *a;

    lng::TokenBuffer buffer = lng::Lex(*reporter, str);
    for (auto tkn : buffer.getTokens())
      lng::PrintToken(tkn, buffer);
  }
}

int main(int argc, const char** argv)
{
  // Register to print a message on allocation failure
  mem::global_on_null([]() noexcept { io::print_fatal("Compiler could not allocate enough memory!"); });
  // Parse command line arguments
  cl::parse_command_line_options<CMDs>(argc, argv);
  
  // On debug configuration, runs tests
  clt::run_tests();

  if (InputFile.empty())
    REPL();
  else
    io::print_warn("Transpilation is not implemented...");  

  if (WaitForUserInput)
    io::press_to_continue();
}