#ifndef DEBUGROUTINES_H
#define DEBUGROUTINES_H

// dirty, but I get OS header includes this way
#include "JGE.h"

#include <ostream>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <string>
#include <sstream>

using namespace std;

template <class T>
std::string ToHex(T* pointer)
{
    std::ostringstream stream;
    stream << hex << showbase << setfill('0') << setw(8) << (uint64_t) pointer;
    return stream.str();
}

#ifdef LINUX
#define OutputDebugString(val) (std::cerr << val);
#endif

#ifdef _DEBUG
#if defined (WIN32) || defined (LINUX)

#ifdef QT_CONFIG
#define DebugTrace(inString)								\
{															\
  std::ostringstream stream;								\
  stream << inString << std::endl;					    \
  qDebug("%s", stream.str().c_str());	                        \
}
#elif defined (ANDROID)
#include <android/log.h>
#define DebugTrace(inString)								\
{															\
  std::ostringstream stream;								\
  stream << inString;					    \
  __android_log_write(ANDROID_LOG_DEBUG, "Wagic", stream.str().c_str());\
}
#else
#define DebugTrace(inString)								\
{															\
	std::ostringstream stream;								\
	stream << inString << std::endl;					    \
  OutputDebugString(stream.str().c_str());  \
}
#endif // QT_CONFIG
#endif // Win32, Linux
#endif //#ifdef _DEBUG

#if defined (DEBUG)
#ifndef DebugTrace
#define DebugTrace(inString)								\
{															\
  std::cerr << inString << std::endl;					    \
}
#endif //DEBUG
#endif

#ifndef DebugTrace
#define DebugTrace(inString)	(void (0))
#endif

#endif // DEBUGROUTINES_H
