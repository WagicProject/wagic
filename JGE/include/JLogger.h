#ifndef _JLOGGER_H_
#define _JLOGGER_H_
//logging facility
//#define DOJLOG

#ifdef DOJLOG
#define JLOG(x) JLogger::Log(x);
#else
#define JLOG(x) {};
#endif

#define JGE_LOG_FILE "jge_debug.txt"

class JLogger{
 public:
  static void Log(const char * text);
};

#endif