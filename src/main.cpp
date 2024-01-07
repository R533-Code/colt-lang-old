#include <util/colt_pch.h>
#include "args.h"
#include "structs/unique_ptr.h"

using namespace clt;

int main(int argc, char** argv)
{
  mem::global_on_null([]() noexcept { io::print_fatal("Could not allocate memory!"); });
  cl::parse_command_line_options<CMDs>(argc, argv);

  mem::StackAllocator<1024_B> stack;

  auto a = make_local_unique<u32>(stack, 100);
  io::print_message("{}: {}", a.get(), *a);

  io::print_message("Hello COLT!\nv{} on {} ({})",
    COLT_VERSION_STRING, COLT_OS_STRING, COLT_CONFIG_STRING);
  
  if (WaitForUserInput)
    (void)getchar();
}