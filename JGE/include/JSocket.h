#ifndef _JSOCKET_H_
#define _JSOCKET_H_

#include <queue>
using namespace std;

//TODO config ?
#define SERVER_PORT 20666

class JSocket{
public:
  queue<char> received_data;
  queue<char> tosend_data;
  static JSocket * mInstance;
  int start_server(const char *szIpAddr);
  int start_client(const char *szIpAddr);
  
  JSocket();
  ~JSocket();
  static int connected;
private:
  void init();  
  void readWrite(int sock);
#if defined (WIN32) || defined (LINUX)
#else
  int make_socket(uint16_t port);
#endif
};

#endif
