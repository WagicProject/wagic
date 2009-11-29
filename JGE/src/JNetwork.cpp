/*
 PSP Network function calls


*/


#if defined (WIN32) || defined (LINUX)
#else
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


#include "../include/JNetwork.h"
#include "../include/JSocket.h"

JNetwork* JNetwork::mInstance = NULL;
string JNetwork::serverIP = "";
string JNetwork::error = "";

JNetwork * JNetwork::GetInstance(){
  if (!mInstance) mInstance = new JNetwork();
  return mInstance;
}

void JNetwork::EndInstance(){
  SAFE_DELETE(mInstance);
}



#if defined (WIN32) || defined (LINUX)
  DWORD JNetwork::netthread = 0;
  int JNetwork::connected_to_ap = 1;
#else
  int JNetwork::connected_to_ap = 0;
  int JNetwork::netthread = 0;
#endif

int JNetwork::isConnected(){
  if (connected_to_ap !=1) return 0;
  return JSocket::connected;
}

JNetwork::JNetwork(){
}

int JNetwork::receive(char * buffer, int length){
  JSocket * socket = JSocket::mInstance;
  if (!socket) return 0;
  int size = 0;
  while(!socket->received_data.empty() && size < length){
    buffer[size] = socket->received_data.front();
    socket->received_data.pop();
    size++;
  }
  return size;
}

int JNetwork::send(char * buffer, int length){
  JSocket * socket = JSocket::mInstance;
  if (!socket) return 0;
  for (int i = 0; i < length; i++){
    socket->tosend_data.push(buffer[i]);
  }
  return length;
}



#if defined (WIN32)
int JNetwork::net_thread(void* param)
{
  do
    {
      JSocket::mInstance = new JSocket();
      JSocket * s = JSocket::mInstance;
      if (JNetwork::serverIP.size()){
	OutputDebugString(JNetwork::serverIP.c_str());
	s->start_client(JNetwork::serverIP.c_str());
      }else{
	s->start_server(""); //IP address useless for server ?
      }
    }
  while(0);

  return 0;
}
int JNetwork::connect(string serverIP){
  if(netthread) return 0;

  JNetwork::serverIP = serverIP;
  /* Create a user thread to do the real work */
  HANDLE hthread;
  hthread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)net_thread,0,0,&netthread);
  return netthread;
}

#elif defined (LINUX)
void* JNetwork::net_thread(void* param __attribute__((unused)))
{
  do
    {
      JSocket::mInstance = new JSocket();
      JSocket * s = JSocket::mInstance;
      if (JNetwork::serverIP.size())
	s->start_client(JNetwork::serverIP.c_str());
      else
	s->start_server(""); //IP address useless for server ?
    }
  while (0);
  return NULL;
}

int JNetwork::connect(string serverIP)
{
  if (netthread) return 0;
  JNetwork::serverIP = serverIP;
  return pthread_create(&netthread, NULL, net_thread, NULL);
}

#else
int net_thread(SceSize args, void *argp)
{
	do
	{
    JSocket::mInstance = new JSocket();
    JSocket * s = JSocket::mInstance;
    if (JNetwork::serverIP.size()){
			s->start_client(JNetwork::serverIP.c_str());

    }else{
			// connected, get my IPADDR and run test
			SceNetApctlInfo szMyIPAddr;
      if (sceNetApctlGetInfo(8, &szMyIPAddr) != 0){
      }else{
			  s->start_server(szMyIPAddr.ip);
      }

		}
	}
	while(0);

	return 0;
}


int JNetwork::connect(string serverIP){
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

  return 0;
}


/* Connect to an access point */
int JNetwork::connect_to_apctl(int config)
{
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

	return 1;
}
#endif
