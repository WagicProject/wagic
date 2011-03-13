#ifndef _JSOCKET_H_
#define _JSOCKET_H_

#include <queue>
#include "Threading.h"
using namespace std;

class JSocket{
public:
    typedef enum {
        // no network available currently from the device
        NOT_AVAILABLE,
        // network available but disconnected from another socket
        DISCONNECTED,
        // connected with another socket
        CONNECTED,
        // some fatal problem
        FATAL_ERROR,
    } SOCKET_STATE;


  // Server creation
  JSocket(string ipAddr);
  // Client creation
  JSocket();
  ~JSocket();

  JSocket* Accept();
  int Read(char* buff, int size);
  int Write(char* buff, int size);
  bool isConnected() { return state == CONNECTED; };
  void Disconnect();

private:
  // socket creation when server accepts a connection
  JSocket(int fd);
  // convert the socket into non-blocking state
  bool SetNonBlocking(int sock);
  // socket handle
#ifdef WIN32
  SOCKET mfd;
#elif LINUX
  int mfd;
#endif
  // socket state
  SOCKET_STATE state;
};

#endif
