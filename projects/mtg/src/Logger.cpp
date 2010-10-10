#include "../include/config.h"
#include "../include/DebugRoutines.h"
#include "../include/Logger.h"
#ifdef DOLOG

#include <iostream>
#include <fstream>
using namespace std;

#if defined (WIN32)
#include <windows.h>
#endif

void Logger::Log(const char * text){
  ofstream file (LOG_FILE,ios_base::app);
  if (file){
    file << text;
    file << "\n";
    file.close();
  }

  DebugTrace(text);
}

#endif