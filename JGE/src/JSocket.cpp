// need a complete rewrite to comply to the new interface
#ifdef NETWORK_SUPPORT

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspsdk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <pspnet_resolver.h>
#include <psputility_netmodules.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#endif

#include "../include/JGE.h"
#include "../include/JSocket.h"

JSocket::JSocket(){
#ifdef NETWORK_SUPPORT
	sceUtilityLoadNetModule(1);
	sceUtilityLoadNetModule(3);
#endif
}

JSocket::JSocket(string ipAddr)
{
}

JSocket::JSocket(int fd)
{
}

JSocket::~JSocket(){
#ifdef NETWORK_SUPPORT
  	pspSdkInetTerm();
		sceNetApctlDisconnect();
		sceNetApctlTerm();
#endif        
}

void JSocket::Disconnect()
{
}

JSocket* JSocket::Accept()
{
	return 0;
}

int JSocket::Read(char* buff, int size)
{
	return 0;
}

int JSocket::Write(char* buff, int size)
{
	return 0;
}


#if 0
int JSocket::make_socket(uint16_t port)
{
	int sock = -1;
 #ifdef NETWORK_SUPPORT   
	int ret;
	struct sockaddr_in name;

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		return -1;
	}

	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	ret = bind(sock, (struct sockaddr *) &name, sizeof(name));
	if(ret < 0)
	{
		return -1;
	}
#endif
	return sock;
}


void JSocket::readWrite(int val){
#ifdef NETWORK_SUPPORT
	char data[1024];
	int readbytes;
  readbytes = read(val, data, sizeof(data));
	if(readbytes < 0)
	{
		printf("Socket %d closed\n", val);
		close(val);
	}
	else
	{
    for (int i = 0; i < readbytes; i++){
      received_data.push(data[i]);
    }
	}

  //Write
  unsigned int size = 0;
  while(!tosend_data.empty()){
    size++;
    if (size > sizeof(data)){
      write(val,data,sizeof(data));
      size = 0;
    }
    data[size-1] = tosend_data.front();
    tosend_data.pop();
  }
  if (size) write(val,data,size);
 #endif 
}

/* Start a client */
int JSocket::start_client(const char *szIpAddr){
#ifdef NETWORK_SUPPORT
 int sock;
  sockaddr_in addrListen;
  int error;

  sock = sceNetInetSocket(AF_INET, SOCK_STREAM, 0);
 if (sock <= 0){
  printf("socket returned $%x\n", sock);
  sceKernelDelayThread(500*1000);
  return sock;
 }

  addrListen.sin_family = AF_INET;
  addrListen.sin_addr.s_addr  = inet_addr(szIpAddr);
  addrListen.sin_port = htons(SERVER_PORT);

  int err = sceNetInetConnect(sock, (sockaddr *)&addrListen, sizeof(addrListen));
  if (err != 0){
   printf("Unable to connect!\n");
   printf("connect returned $%x\n", err);
   printf("  errno=$%x\n", sceNetInetGetErrno());
   sceKernelDelayThread(500*1000);
   return err;
 }

 connected = 1;

 while(1){
   readWrite(sock);
 }
 #endif
 return 0;
}

/* Start a server */
int JSocket::start_server(const char *szIpAddr)
{
#ifdef NETWORK_SUPPORT
	int ret;
	int sock;
	int _new = -1;
	struct sockaddr_in client;
	size_t size;
	int readbytes;

	fd_set set;
	fd_set setsave;

	/* Create a socket for listening */
	sock = make_socket(SERVER_PORT);
	if(sock < 0)
	{
		printf("Error creating server socket\n");
		return sock;
	}

	ret = listen(sock, 1);
	if(ret < 0)
	{
		printf("Error calling listen\n");
		return ret;
	}

	printf("Listening for connections ip %s port %d\n", szIpAddr, SERVER_PORT);

	FD_ZERO(&set);
	FD_SET(sock, &set);
	setsave = set;

	while(1)
	{
		int i;
		set = setsave;
		if(select(FD_SETSIZE, &set, NULL, NULL, NULL) < 0)
		{
			printf("select error\n");
			return -1;
		}

		for(i = 0; i < FD_SETSIZE; i++)
		{
			if(FD_ISSET(i, &set))
			{
				int val = i;

				if(val == sock)
				{
					_new = accept(sock, (struct sockaddr *) &client, &size);
					if(_new < 0)
					{
						printf("Error in accept %s\n", strerror(errno));
						close(sock);
						return _new;
					}

					printf("New connection %d from %s:%d\n", val,
							inet_ntoa(client.sin_addr),
							ntohs(client.sin_port));

					write(_new, "123567", strlen("123567"));

					FD_SET(_new, &setsave);
				}
				else
				{
					readWrite(val);
				}
			}
		}
	}

	close(sock);
#endif    
	return 0;
}

#endif //NETWORK_SUPPORT


