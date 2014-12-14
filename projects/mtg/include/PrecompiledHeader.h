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

#ifdef WP8
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif

#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#if defined(_XBOX_ONE) && defined(_TITLE)
#include <d3d11_x.h>
#define DCOMMON_H_INCLUDED
#define NO_D3D11_DEBUG_NAME
#else
#include <d3d11_1.h>
#endif

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXCollision.h>

#include <algorithm>
#include <array>
#include <exception>
#include <malloc.h>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

// VS 2010's stdint.h conflicts with intsafe.h
#pragma warning(push)
#pragma warning(disable : 4005)
#include <stdint.h>
#include <intsafe.h>
#pragma warning(pop)

#include <wrl.h>
#include "..\extra\dirent.h"

namespace DirectX
{
#if (DIRECTX_MATH_VERSION < 305) && !defined(XM_CALLCONV)
#define XM_CALLCONV __fastcall
	typedef const XMVECTOR& HXMVECTOR;
	typedef const XMMATRIX& FXMMATRIX;
#endif
}

#else //WP8
#include <boost/shared_ptr.hpp>
#include <dirent.h>
#endif

#include "JGE.h"
#include "JFileSystem.h"
#include "JLogger.h"

#include "GameOptions.h"

#if defined (WP8) || defined (IOS) || defined (ANDROID) || defined (QT_CONFIG) || defined (SDL_CONFIG)
#define TOUCH_ENABLED
#endif

#endif //PRECOMPILEDHEADER_H
