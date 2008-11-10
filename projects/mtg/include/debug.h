#ifndef _DEBUG_H_
#define _DEBUG_H_

#if defined (WIN32) || defined (LINUX)
#define TESTSUITE 1
#endif

#ifdef _DEBUG
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

#endif
