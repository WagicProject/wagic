#ifndef _JLOGGER_H_
#define _JLOGGER_H_
//logging facility
//#define DOLOG

#ifdef DOLOG
#define LOG(x) JLogger::Log(x);
#else
#define LOG(x) {};
#endif

// saving myself the pain of search/replace
#define JLOG(x) LOG(x)

#define LOG_FILE "debug.txt"

class JLogger{
 public:
  static void Log(const char * text);

  JLogger(const char* text);
  ~JLogger();

  const char* mText;
};

#endif