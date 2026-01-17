#ifndef RLD_LIBS_H
#define RLD_LIBS_H

#if RL_FAST_COMPILE
// NOTE(luca): If fast compile mode is on, the implementation will be compiled to a separate translation unit (see `../build/rl_libs.o` in `build.sh`).
//
# include "lib/stb_image.h"
# include "lib/stb_truetype.h"

#else

#include "base/base_core.h"

NO_WARNINGS_BEGIN
# define STB_IMAGE_IMPLEMENTATION
# define STB_TRUETYPE_IMPLEMENTATION
# define RL_FONT_IMPLEMENTATION
# include "lib/stb_image.h"
# include "lib/stb_truetype.h"
NO_WARNINGS_END
#endif // RL_FAST_COMPILE

#endif // RLD_LIBS_H
