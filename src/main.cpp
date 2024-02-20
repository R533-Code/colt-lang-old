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
#include "err/composable_reporter.h"

using namespace clt;

void test()
{
  using namespace lng;

  auto reporter = lng::make_error_reporter<lng::ConsoleReporter>();
  while (true)
  {
    io::print<"">("Enter line to lex: ");
    auto a = String::getLine(64, false);
    if (a.is_error())
      return;
    const auto& str = *a;
  
    TokenBuffer buffer = lng::Lex(*reporter, str);
    for (auto tkn : buffer.getTokens())
      io::print("{}: line {} [{}:{}]",
        tkn.getLexeme(), buffer.getLine(tkn),
        buffer.getColumn(tkn), buffer.getSize(tkn));
  }
}

int main(int argc, const char** argv)
{
  mem::global_on_null([]() noexcept { io::print_fatal("Could not allocate memory!"); });
  cl::parse_command_line_options<CMDs>(argc, argv);
  
  test();

  if (InputFile.empty())
    io::print_warn("REPL is not implemented...");
  else
    io::print_warn("Transpilation is not implemented...");  

  if (WaitForUserInput)
    io::press_to_continue();
}