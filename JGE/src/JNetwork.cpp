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

map<string, processCmd> JNetwork::sCommandMap;

bool JNetwork::isConnected(){
  if (connected_to_ap !=1) return false;
  return socket->isConnected();
}

JNetwork::JNetwork()
  : mpWorkerThread(NULL)
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

bool JNetwork::sendCommand(string xString)
{
  string aString = xString;
  boost::mutex::scoped_lock l(sendMutex);
  if(!socket) {
    DebugTrace("sendCommand failed: no sockeet");
    return false;
  }
  aString = aString + "Command";
  if(sCommandMap.find(aString) == sCommandMap.end()) {
    DebugTrace("sendCommand failed: command not registered");
    return false;
  }
  aString = aString + "\n";

  toSend << aString;

  return true;
}

void JNetwork::registerCommand(string command, processCmd processCommand, processCmd processResponse)
{
  sCommandMap[command + "Command"] = processCommand;
  sCommandMap[command + "Response"] = processResponse;
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
    char buff[1024];
    {
      boost::mutex::scoped_lock l(pThis->receiveMutex);
      int len =  pSocket->Read(buff, sizeof(buff));
      if(len) {
        DebugTrace("receiving " << len << " bytes : " << buff);
        pThis->received << buff;
      }
      // Checking for some command to execute
      size_t found = pThis->received.str().find("Command");
      if(found != string::npos)
      {
        map<string, processCmd>::iterator ite = sCommandMap.find((pThis->received.str()).substr(0, found) + "Command");
        if(ite != sCommandMap.end())
        {
          DebugTrace("begin of command received : "<< pThis->received.str() );
          DebugTrace("begin of command toSend : "<< pThis->toSend.str() );

          boost::mutex::scoped_lock l(pThis->sendMutex);
          pThis->toSend << pThis->received.str().substr(0, found) + "Response ";
          pThis->received.str("");
          processCmd theMethod = (ite)->second;
          theMethod(pThis->received, pThis->toSend);

          DebugTrace("end of command received : "<< pThis->received.str() );
          DebugTrace("end of command toSend : "<< pThis->toSend.str() );
        }
      }
      // Checking for some response to execute
      found = pThis->received.str().find("Response");
      if(found != string::npos)
      {
        map<string, processCmd>::iterator ite = sCommandMap.find((pThis->received.str()).substr(0, found) + "Response");
        if(ite != sCommandMap.end())
        {
          DebugTrace("begin of response received : "<< pThis->received.str() );
          DebugTrace("begin of response toSend : "<< pThis->toSend.str() );

          boost::mutex::scoped_lock l(pThis->sendMutex);
          string aString;
          pThis->received >> aString;
          processCmd theMethod = (ite)->second;
          theMethod(pThis->received, pThis->toSend);
          pThis->received.str("");

          DebugTrace("end of response received : "<< pThis->received.str() );
          DebugTrace("end of response toSend : "<< pThis->toSend.str() );
        }
      }
    }

    boost::mutex::scoped_lock l(pThis->sendMutex);
    if(!pThis->toSend.str().empty())
    {
      DebugTrace("sending  " << pThis->toSend.str().size() << " bytes : " << pThis->toSend.str());
      pSocket->Write((char*)pThis->toSend.str().c_str(), pThis->toSend.str().size()+1);
      pThis->toSend.str("");
    }
  }

  DebugTrace("Quitting Thread");
}

#if defined (WIN32) || defined (LINUX)
int JNetwork::connect(string ip)
{
  if (mpWorkerThread) return 0;
  serverIP = ip;
  mpWorkerThread = new boost::thread(JNetwork::ThreadProc, this);
  return 42;
}

#else

int JNetwork::connect(string serverIP){
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
