#ifndef _DEBUG_H_
#define _DEBUG_H_

#if WIN32
#define snprintf sprintf_s
#endif 

#if (defined (WIN32) || defined (LINUX)) && defined (_DEBUG)
#define TESTSUITE 1

#else
#define OutputDebugString(val) {}
#endif




#include "limits.h"


#if defined (_DEBUG) && defined (WIN32)
#include "crtdbg.h"
#define NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define NEW new
#endif

#ifdef QT_CONFIG
#include <QtGlobal>
#define OutputDebugString(val) qDebug(val)
#else
#ifdef LINUX
#ifdef _DEBUG
#define OutputDebugString(val) (std::cerr << val);
#else
#define OutputDebugString(val) {}
#endif
#endif
#endif


#ifndef RESPATH
#define RESPATH "Res"
#endif

#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

// Debug options - comment/uncomment as needed
//#define DEBUG_CACHE
#ifdef _DEBUG
//#define RENDER_AI_STATS
#endif

#endif
