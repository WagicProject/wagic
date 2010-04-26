#include "../include/JLogger.h"
#include <iostream>
#include <fstream>
using namespace std;

#if defined (WIN32)
#include <windows.h>
#endif

void JLogger::Log(const char * text){
  ofstream file (JGE_LOG_FILE,ios_base::app);
  if (file){
    file << text;
    file << "\n";
    file.close();
  }
#if defined (WIN32) || defined (LINUX)
  OutputDebugString(text);
  OutputDebugString("\n");
#else
  printf(text);
  printf("\n");
#endif

}