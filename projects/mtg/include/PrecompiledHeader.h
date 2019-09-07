#ifndef PRECOMPILEDHEADER_H
#define PRECOMPILEDHEADER_H


#define __STDC_LIMIT_MACROS

#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#if (__cplusplus > 199711L) ||  (_MSC_VER >= 1700)
#include <cinttypes>
#endif

#include "config.h"
#include "DebugRoutines.h"

#include <assert.h>

#include "JGE.h"
#include "JFileSystem.h"
#include "JLogger.h"

#include "GameOptions.h"

#if defined (WP8) || defined (IOS) || defined (ANDROID) || defined (QT_CONFIG) || defined (SDL_CONFIG)
#define TOUCH_ENABLED
#endif

#endif //PRECOMPILEDHEADER_H
