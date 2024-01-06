#include <util/colt_config.h>
#include <io/print.h>

using namespace clt;

int main()
{
  io::print_message("Hello COLT!\nv{} on {} ({})",
    COLT_VERSION_STRING, COLT_OS_STRING, COLT_CONFIG_STRING);
}