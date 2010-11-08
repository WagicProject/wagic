#ifndef _LOGGER_H
#define _LOGGER_H_

//TODO Remove this and use the jge logging facility (same system)
//#define DOLOG

#ifdef DOLOG
#define LOG_FILE "debug.txt"

class Logger{
 public:
  static void Log(const char * text);
};
#define LOG(x) Logger::Log(x);
#else
#define LOG(x)
#endif

#endif
