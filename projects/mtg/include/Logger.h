#ifndef _LOGGER_H
#define _LOGGER_H_




#ifdef DOLOG
#define LOG(x) Logger::Log(x);
#else
#define LOG(x)
#endif

#define LOG_FILE "Res/debug.txt"

class Logger{
public:
	static void Log(char * text);
};

#endif
