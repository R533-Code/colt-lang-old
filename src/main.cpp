#include <util/colt_pch.h>
#include "args.h"

using namespace clt;

int main(int argc, char** argv)
{
  cl::parse_command_line_options<CMDs>(argc, argv);

  io::print_message("Hello COLT!\nv{} on {} ({})",
    COLT_VERSION_STRING, COLT_OS_STRING, COLT_CONFIG_STRING);
  
  if (WaitForUserInput)
    (void)getchar();
}