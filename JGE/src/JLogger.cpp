#include "../include/JLogger.h"
#include "../include/JGE.h"
#include "../include/DebugRoutines.h"

#include <fstream>

string JLogger::lastLog = "";
int JLogger::lastTime = 0;

void JLogger::Log(const char * text){
#ifdef DOLOG
  std::ofstream file(LOG_FILE, std::ios_base::app);
  std::stringstream out;
  int newTime = JGEGetTime();
  out << newTime << "(+" << newTime - lastTime << ") :" << text;
  if (file){
    file << out.str();
    file << "\n";
    file.close();
    lastTime = newTime;
  }

  DebugTrace(out.str());
#endif
  lastLog = text;
}

void JLogger::Log(std::string text){
    Log(text.c_str());
}

JLogger::JLogger(const char* text) : mText(text)
{
#ifdef DOLOG
  std::ostringstream stream;
  stream << mText << ": Start";
  JLogger::Log(stream.str().c_str());
#endif
}

JLogger::~JLogger()
{
#ifdef DOLOG
  std::ostringstream stream;
  stream << mText << ": End";
  JLogger::Log(stream.str().c_str());
#endif
}

