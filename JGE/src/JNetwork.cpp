/*
 PSP Network function calls


*/

#include "../include/DebugRoutines.h"
#include "../include/JNetwork.h"

#if defined (WIN32) || defined (LINUX)
#else
#ifdef NETWORK_SUPPORT

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <psputility.h> 
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <pspnet_resolver.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#endif

#endif

#include <sstream>
#include "../include/JSocket.h"

map<string, JNetwork::CommandStruc> JNetwork::sCommandMap;

bool JNetwork::isConnected(){
  if (connected_to_ap !=1 || !socket) return false;
  return socket->isConnected();
}

void JNetwork::getServerIp(string& aString)
{
#ifdef WIN32
    char ac[80];
    if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR) {
        DebugTrace("Error " << WSAGetLastError() <<
                " when getting local host name.\n");
		return;
    }
    DebugTrace( "Host name is " << ac << ".\n");

    struct hostent *phe = gethostbyname(ac);
    if (phe == 0) {
        DebugTrace("Yow! Bad host lookup.\n");
		return;
    }

    for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
        struct in_addr addr;
        memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
        aString = inet_ntoa(addr);
    }
#endif
}


JNetwork::JNetwork()
  : mpWorkerThread(NULL), socket(0)
{
#if (defined WIN32) || (defined LINUX)
  connected_to_ap = 1;
#else
  connected_to_ap = 0;
#endif

}

JNetwork::~JNetwork()
{
  if(mpWorkerThread) {
    socket->Disconnect();
    mpWorkerThread->join();
    delete mpWorkerThread;
  }
  if(socket)
    delete socket;
}

bool JNetwork::sendCommand(const string& xString, const string& payload, const string& suffix)
{
  string aString;
  boost::mutex::scoped_lock l(sendMutex);
  if(!socket) {
    DebugTrace("sendCommand failed: no socket");
    return false;
  }
  aString = xString + suffix;
  if(sCommandMap.find(aString) == sCommandMap.end()) {
    DebugTrace("sendCommand failed: command not registered");
    return false;
  }
  aString = "<" + aString + ">" + "\n" + payload + "\n" + "<" + aString + "/>";

  toSend << aString;

  return true;
}

void JNetwork::registerCommand(string command, void* object, processCmd processCommand, processCmd processResponse)
{
	CommandStruc struc;
	struc.object = object;
	struc.command = command;
	struc.processCommand = processCommand;

	sCommandMap[command + "Request"] = struc;
	struc.processCommand = processResponse;
	sCommandMap[command + "Response"] = struc;
}

void JNetwork::Update()
{
	boost::mutex::scoped_lock r(receiveMutex);
    // Checking for some command to execute
	size_t begin_start_tag = received.str().find("<");
	size_t end_start_tag = received.str().find(">");
	string command = received.str().substr(begin_start_tag+1, end_start_tag-(begin_start_tag+1));
	size_t begin_end_tag = received.str().find(command + "/>");
	size_t end_end_tag = received.str().find("/>");
    if(begin_start_tag != string::npos && begin_end_tag != string::npos )
    {
		map<string, CommandStruc>::iterator ite = sCommandMap.find(command);
		if(ite != sCommandMap.end())
		{
			DebugTrace("begin of command received : " + received.str() );
			DebugTrace("begin of command toSend : " + toSend.str() );

			processCmd theMethod = (ite)->second.processCommand;
			stringstream input(received.str().substr(end_start_tag+1, (begin_end_tag-1)-(end_start_tag+1)));
			stringstream output;
			theMethod((ite)->second.object, input, output);
			string aString = received.str().substr(end_end_tag+2, string::npos);
			received.str(aString);
			if(output.str().size())
				sendCommand((ite)->second.command, output.str(), "Response");

			DebugTrace("end of command received : "<< received.str() );
			DebugTrace("end of command toSend : "<< toSend.str() );
		}
    }
}

void JNetwork::ThreadProc(void* param)
{
  JNetwork* pThis = reinterpret_cast<JNetwork*>(param);
  JSocket* pSocket = NULL;
  if (pThis->serverIP.size()) {
    DebugTrace("Starting Client Thread");
    pThis->socket = new JSocket(pThis->serverIP);
    if(pThis->socket->isConnected())
      pSocket = pThis->socket;
  } else {
    DebugTrace("Starting Server Thread");
    pThis->socket = new JSocket();
    // Wait for some client
    pSocket = pThis->socket->Accept();
  }

  while(pSocket && pSocket->isConnected()) {
    char buff[4096];
    {
      int len =  pSocket->Read(buff, sizeof(buff));
      if(len > 0) {
		boost::mutex::scoped_lock l(pThis->receiveMutex);
        DebugTrace("receiving " << len << " bytes : " << buff);
        pThis->received.str(pThis->received.str() + buff);
        DebugTrace("received " << pThis->received.str().size() << " bytes : " << pThis->received.str());
      }
    }
	{
		boost::mutex::scoped_lock l(pThis->sendMutex);
		if(!pThis->toSend.str().empty())
		{
		  DebugTrace("sending  " << pThis->toSend.str().size() << " bytes : " << pThis->toSend.str());
		  pSocket->Write((char*)pThis->toSend.str().c_str(), pThis->toSend.str().size()+1);
		  pThis->toSend.str("");
		}
	}
    boost::this_thread::sleep(boost::posix_time::milliseconds(1));
  }

  DebugTrace("Quitting Thread");
}

#if defined (WIN32) || defined (LINUX)
int JNetwork::connect(const string& ip)
{
  if (mpWorkerThread) return 0;
  serverIP = ip;
  mpWorkerThread = new boost::thread(JNetwork::ThreadProc, this);
  return 42;
}

#else

int JNetwork::connect(const string& serverIP){
#ifdef NETWORK_SUPPORT
  int err;
  char buffer[4096];
  if(netthread) return 0;


	sceUtilityLoadNetModule(1);
sceUtilityLoadNetModule(3);

	if((err = pspSdkInetInit())){
		sprintf(buffer, "JGE Error, could not initialise the network %08X", err);
    printf(buffer); printf("\n");
    error = buffer;
		return err;
	}

  if(JNetwork::connect_to_apctl(1)){
    JNetwork::serverIP = serverIP;
	  /* Create a user thread to do the real work */
	   netthread = sceKernelCreateThread("net_thread", net_thread, 0x18, 0x10000, PSP_THREAD_ATTR_USER, NULL);

	  if(netthread < 0)
	  {
		  printf("Error, could not create thread\n");
		  sceKernelSleepThread();
	  }

	  sceKernelStartThread(netthread, 0, NULL);
    return netthread;
  }
#endif
  return 0;
}


/* Connect to an access point */
int JNetwork::connect_to_apctl(int config)
{
#ifdef NETWORK_SUPPORT
	int err;
	int stateLast = -1;
  char buffer[4096];

	/* Connect using the first profile */
	err = sceNetApctlConnect(config);
	if (err != 0)
	{
		sprintf(buffer, "JGE: sceNetApctlConnect returns %08X", err);
    printf(buffer);printf("\n");
    error = buffer;
		return 0;
	}

	sprintf(buffer,"JGE: Connecting...");
  printf(buffer);printf("\n");
  error = buffer;
	while (1)
	{
		int state;
		err = sceNetApctlGetState(&state);
		if (err != 0)
		{
			sprintf(buffer,"JGE: sceNetApctlGetState returns $%x", err);
        printf(buffer);printf("\n");
      error = buffer;
			break;
		}
		if (state > stateLast)
		{
			sprintf(buffer, "  connection state %d of 4", state);
        printf(buffer);printf("\n");
        error = buffer;
			stateLast = state;
		}
    if (state == 4){
      connected_to_ap = 1;
			break;  // connected with static IP
    }
		// wait a little before polling again
		sceKernelDelayThread(50*1000); // 50ms
	}
	printf("JGE: Connected!\n");

	if(err != 0)
	{
		return 0;
	}
#endif
	return 1;
}
#endif
