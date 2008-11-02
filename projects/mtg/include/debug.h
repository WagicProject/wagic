#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef _DEBUG
#define NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define TESTSUITE 1
#else
#define NEW new
#endif

#ifdef LINUX
  #ifdef _DEBUG
    #define OutputDebugString(val) (std::cerr << val);
  #else
    #define OutputDebugString(val)
  #endif
#endif

#endif
