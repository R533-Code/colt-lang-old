#include <util/colt_pch.h>
#include "util/colt_config.h"

using namespace clt;

int main()
{
  io::print_message("Hello COLT!\nv{} on {} ({})",
    COLT_VERSION_STRING, COLT_OS_STRING, COLT_CONFIG_STRING);
}