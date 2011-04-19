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
    stream << hex << showbase << setfill('0') << setw(8) << (int) pointer;
    return stream.str();
}

#if defined (WIN32) || defined (LINUX) 
#ifdef _DEBUG

#ifndef QT_CONFIG
#define DebugTrace(inString)								\
{															\
	std::ostringstream stream;								\
	stream << inString << std::endl;					    \
	OutputDebugString(stream.str().c_str());	            \
}
#else
#define DebugTrace(inString)								\
{															\
	std::ostringstream stream;								\
	stream << inString << std::endl;					    \
	qDebug(stream.str().c_str());	                        \
}
#endif //QT_CONFIG

#endif //#ifdef _DEBUG
#endif // Win32, Linux

#if defined (DEBUG)
#ifndef DebugTrace
#define DebugTrace(inString)								\
{															\
  std::cout << inString << std::endl;					    \
}
#endif //DEBUG
#endif

#ifndef DebugTrace
#define DebugTrace(inString)	(void (0))
#endif

#endif // DEBUGROUTINES_H
