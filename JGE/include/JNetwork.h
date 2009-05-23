#ifndef _JNETWORK_H_
#define _JNETWORK_H_

#ifdef WIN32
#elif defined (LINUX)
#else
#endif

#include "JGE.h"
#include <string>
using namespace std;

class JNetwork{
private:
  static JNetwork * mInstance;
  static int connected_to_ap;
public:
  JNetwork();
  static JNetwork * GetInstance();
  static void EndInstance();
  static string serverIP;
  int receive(char * buffer, int length);
  int send(char * buffer, int length);
  int connect(string serverIP = "");
  static int isConnected();
#if defined (WIN32)
  static int net_thread(void* param);
#elif defined (LINUX)
  static void* net_thread(void* param);
#else
  static int net_thread(SceSize args, void *argp);
  static int connect_to_apctl(int config);
#endif

private:

#if defined (WIN32)
  static DWORD netthread;
#elif defined (LINUX)
  static pthread_t netthread;
#else
  static int netthread;
#endif

};

#endif
