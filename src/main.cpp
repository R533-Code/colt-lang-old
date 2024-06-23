/*****************************************************************/ /**
 * @file   main.cpp
 * @brief  Starting point of the compiler.
 *
 * @author RPC
 * @date   February 2024
 *********************************************************************/
#include "common/colt_pch.h"
#include "args.h"
#include "test/run_tests.h"
#include "ast/parsed_program.h"

using namespace clt;

void REPL()
{
  using namespace lng;

  io::print_warn("REPL is not implemented...");

  auto reporter    = lng::make_error_reporter<lng::ConsoleReporter>();
  const auto& warn = GlobalWarnFor;
  Vector<std::filesystem::path> includes = {};
  while (true)
  {
    io::print<"">("{}>>>{} ", io::BrightCyanF, io::Reset);
    auto a = String::getLine(64, false);
    if (a.is_error())
      return;
    const auto& str = *a;
    auto program    = ParsedProgram{*reporter, StringView{str}, includes, warn};
  }
}

int main(int argc, const char** argv)
{
  // Register to print a message on allocation failure
  mem::global_on_null(
      []() noexcept
      { io::print_fatal("Compiler could not allocate enough memory!"); });
  // Parse command line arguments
  cl::parse_command_line_options<CMDs>(argc, argv);

  // On debug configuration, runs tests
  if (RunTests)
    clt::run_tests();
  else
  {
    if (InputFile.empty())
      REPL();
    else
      io::print_warn("Transpilation is not implemented...");
  }

  if (WaitForUserInput)
    io::press_to_continue();
}