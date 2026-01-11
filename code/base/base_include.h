/* date = January 10th 2026 4:10 pm */

#ifndef BASE_INCLUDE_H
#define BASE_INCLUDE_H

#ifdef RL_PERSONAL
# include "base_build.h"
#endif

#include "base_core.h"
#include "base_strings.h"
#include "base_arenas.h"
#include "base_lanes.h"
#include "base_os.h"

#ifdef BASE_IMPLEMENTATION
# include "base_strings.c"
# include "base_arenas.c"
# include "base_lanes.c"
# include "base_os.c"
#endif

#endif //BASE_INCLUDE_H
