#ifndef _DEBUG_H_
#define _DEBUG_H_

#if defined (WIN32) || defined (LINUX)
#define TESTSUITE 1
#endif

#ifdef _DEBUG
//if you get the following error :'_NORMAL_BLOCK' : undeclared identifier,
// try to add #include "crtdbg.h" somewhere in your code before including this file
#define NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define NEW new
#endif

#ifdef LINUX
#ifdef _DEBUG
#define OutputDebugString(val) (std::cerr << val);
#else
#define OutputDebugString(val) {}
#endif
#endif

#ifndef RESPATH
#define RESPATH "Res"
#endif

#endif
