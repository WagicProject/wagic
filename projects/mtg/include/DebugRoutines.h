#ifndef DEBUGROUTINES_H
#define DEBUGROUTINES_H

// dirty, but I get OS header includes this way
#include "JGE.h"

#include <iostream>
#include <stdio.h>
#include <string>
#include <sstream>

#if defined (WIN32) || defined (LINUX)
#ifdef _DEBUG

#define DebugTrace(inString)								\
{																						\
	std::ostringstream stream;								\
	stream << inString << std::endl;					\
	OutputDebugString(stream.str().c_str());	\
}

#endif //#ifdef _DEBUG
#endif // Win32, Linux

#ifndef DebugTrace
#define DebugTrace(inString)	(void (0))
#endif

#endif // DEBUGROUTINES_H