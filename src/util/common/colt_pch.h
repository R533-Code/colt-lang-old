/*****************************************************************/ /**
 * @file   colt_pch.h
 * @brief  Precompiled header containing the most useful includes.
 *
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_PCH
#define HG_COLT_PCH

#include <fmt/format.h>
#include <fmt/chrono.h>
#include <scn/scn.h>
#include <date/date.h>

#include <thread>
#include <mutex>

#include <filesystem>

#include "io/print.h"
#include "io/input.h"

#include "types.h"
#include "bits.h"
#include "colt_config.h"
#include "macros.h"

#include "meta/meta_enum.h"
#include "structs/unique_ptr.h"
#include "structs/vector.h"
#include "structs/static_vector.h"
#include "structs/string.h"
#include "structs/option.h"
#include "structs/expect.h"
#include "structs/map.h"
#include "structs/set.h"

#endif // !HG_COLT_PCH