#ifndef PRECOMPILEDHEADER_H
#define PRECOMPILEDHEADER_H

#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "config.h"
#include "DebugRoutines.h"

#include <assert.h>

#include "JGE.h"
#include "JFileSystem.h"
#include "JLogger.h"

#include "GameOptions.h"

#ifndef WP8
#include <boost/shared_ptr.hpp>
#endif

#if defined (WP8) || defined (IOS) || defined (ANDROID) || defined (QT_CONFIG) || defined (SDL_CONFIG)
#define TOUCH_ENABLED
#endif

#endif //PRECOMPILEDHEADER_H
