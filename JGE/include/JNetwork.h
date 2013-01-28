#ifndef _JNETWORK_H_
#define _JNETWORK_H_

//Network support for PSP
//#define NETWORK_SUPPORT

#include "JGE.h"
#include <string>
#include <map>
using namespace std;
class JSocket;
#include <iostream>
#include <sstream>
#include "Threading.h"

typedef void(*processCmd)(void*, stringstream&, stringstream&);

class JNetwork {
private:
private:
  boost::thread *mpWorkerThread;
	struct CommandStruc{
		void* object;
		string command;
		processCmd processCommand;
	};

  string serverIP;
  int connected_to_ap;
  JSocket* socket;
  boost::mutex sendMutex;
  boost::mutex receiveMutex;
  stringstream received;
  stringstream toSend;
  static map<string, CommandStruc> sCommandMap;

public:
  JNetwork();
  ~JNetwork();
  void Update();
  void getServerIp(string& aString);

  int connect(const string& serverIP = "");
  bool isConnected();
  bool isServer() { return serverIP == ""; };
  static void ThreadProc(void* param);
#if !defined (WIN32) && !defined (LINUX)
  static int connect_to_apctl(int config);
#endif
  bool sendCommand(const string& command, const string& payload = "", const string& suffix = "Request");
  static void registerCommand(string command, void* object, processCmd processRequest, processCmd processResponse);
};

#endif
