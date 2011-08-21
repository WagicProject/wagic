#include "../include/JLogger.h"
#include "../include/DebugRoutines.h"

#include <fstream>

string JLogger::lastLog = "";

void JLogger::Log(const char * text){
#ifdef DOLOG
  std::ofstream file(LOG_FILE, std::ios_base::app);
  if (file){
    file << text;
    file << "\n";
    file.close();
  }

  DebugTrace(text);
#endif
  lastLog = text;
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

