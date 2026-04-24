#include <errno.h>
#ifdef WIN32
#pragma comment(lib,"WSOCK32.LIB")
#include <stdio.h>
#include <conio.h>
#include <winsock.h>
#include <fcntl.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#endif

#include "../../include/JSocket.h"
#include "../../include/DebugRoutines.h"


JSocket::JSocket(string ipAddr)
	: state(NOT_AVAILABLE),
	mfd(-1)
{
	int result;
	struct hostent *hostentptr = 0;
#ifdef WIN32
	SOCKADDR_IN Adresse_Socket_Server;
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested=MAKEWORD(1, 1);
	result = WSAStartup(wVersionRequested,&wsaData);
	if (result != 0)
	{
		DebugTrace("WSAStartup\t");
		return;
	}
#else
	struct sockaddr_in Adresse_Socket_Server;
#endif

	mfd = socket(AF_INET, SOCK_STREAM, 0);
	if(mfd < 0)
		return;
	DebugTrace("Connecting " << ipAddr);

#ifdef WIN32
	unsigned int addr_dest = inet_addr(ipAddr.c_str());
	hostentptr = gethostbyaddr((char*) &addr_dest, 4, AF_INET);
	if (hostentptr == NULL)
	    hostentptr = gethostbyname(ipAddr.c_str());
#else
	hostentptr = gethostbyname(ipAddr.c_str());
#endif
	if (hostentptr == NULL)
	{
		DebugTrace("ERROR, no such host\n");
		return;
	}

#ifdef WIN32
	ZeroMemory( (char*)&Adresse_Socket_Server, sizeof(Adresse_Socket_Server));
#else
	bzero( (char*)&Adresse_Socket_Server, sizeof(Adresse_Socket_Server));
#endif

	Adresse_Socket_Server.sin_family = (*hostentptr).h_addrtype;
	Adresse_Socket_Server.sin_port = htons(SERVER_PORT);
	Adresse_Socket_Server.sin_addr = *((struct in_addr*)(*hostentptr).h_addr);

	result = connect(
		mfd,
		(const struct sockaddr*)&Adresse_Socket_Server,
		sizeof(Adresse_Socket_Server));
	if (result != 0)
	{
		DebugTrace("client connect failed :" << strerror(errno));
		state = FATAL_ERROR;
		return;
	}

	state = CONNECTED;
}

bool JSocket::SetNonBlocking(int sock)
{
#ifndef WIN32
	int opts = fcntl(sock,F_GETFL);
	if (opts < 0)
	{
		perror("fcntl(F_GETFL)");
		return false;
	}
	opts = (opts | O_NONBLOCK);
	if (fcntl(sock,F_SETFL,opts) < 0)
	{
		perror("fcntl(F_SETFL)");
		return false;
	}
#endif
	return true;
}

JSocket::JSocket()
	: state(NOT_AVAILABLE),
	mfd(-1)
{
	int result;
#ifdef WIN32
	SOCKADDR_IN Adresse_Socket_Connection;
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(1,1);
	result = WSAStartup(wVersionRequested,&wsaData);

	if (result != 0)
	{
		DebugTrace("WSAStartup\t");
		return;
	}
#else
	struct sockaddr_in Adresse_Socket_Connection;
#endif

	mfd=socket( AF_INET, SOCK_STREAM,0);

#ifndef WIN32
	int reuse_addr = 1;
	setsockopt(mfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));
#endif

	SetNonBlocking(mfd);

#ifdef WIN32
	ZeroMemory( &Adresse_Socket_Connection,sizeof(Adresse_Socket_Connection));
#else
	bzero( &Adresse_Socket_Connection,sizeof(Adresse_Socket_Connection));
#endif
	Adresse_Socket_Connection.sin_family = AF_INET;
	Adresse_Socket_Connection.sin_port = htons(SERVER_PORT);

	result = ::bind(mfd,
		(struct sockaddr*)&Adresse_Socket_Connection,
		sizeof(Adresse_Socket_Connection));
	if (result != 0)
	{
		state = FATAL_ERROR;
		DebugTrace("bind error:" <<  strerror(errno));
		return;
	}

	result = listen(mfd,1);
	if(result != 0)
	{
		state = FATAL_ERROR;
		DebugTrace("listen error:" << strerror(errno));
		return;
	}

	state = DISCONNECTED;
}

JSocket::JSocket(int fd)
	: state(CONNECTED),
	mfd(fd)
{
}

JSocket::~JSocket()
{
	Disconnect();
#ifdef WIN32
	WSACleanup();
#endif
}

void JSocket::Disconnect()
{
	state = JSocket::DISCONNECTED;
	if(mfd)
	{
#ifdef WIN32
		closesocket(mfd);
#else
		close(mfd);
#endif
		mfd = 0;
	}
}

JSocket* JSocket::Accept()
{
#ifdef WIN32
	SOCKADDR_IN Adresse_Socket_Cliente;
	int Longueur_Adresse;
#else
	struct sockaddr_in Adresse_Socket_Cliente;
	socklen_t Longueur_Adresse;
#endif

	JSocket* socket = NULL;

	while (mfd)
	{
		fd_set set;
		FD_ZERO(&set);
		FD_SET(mfd, &set);
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 1000 * 100;

		int result = select(mfd+1, &set, NULL, NULL, &tv);
		if (result > 0 && FD_ISSET(mfd, &set))
		{
			Longueur_Adresse = sizeof(Adresse_Socket_Cliente);
			int val = accept(
				mfd,
				(struct sockaddr*)&Adresse_Socket_Cliente,
				&Longueur_Adresse);
			DebugTrace("connection on client port "<< ntohs(Adresse_Socket_Cliente.sin_port));

			if (val >= 0)
			{
				state = CONNECTED;
				socket = new JSocket(val);
			}
			break;
		}
	}

	return socket;
}

int JSocket::Read(char* buff, int size)
{
	if (state == CONNECTED)
	{
		fd_set set;
		FD_ZERO(&set);
		FD_SET(mfd, &set);
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 1000 * 100;

		int result = select(mfd+1, &set, NULL, NULL, &tv);
		if (result > 0 && FD_ISSET(mfd, &set))
		{
#ifdef WIN32
			int readbytes = recv(mfd, buff, size, 0);
#else
			int readbytes = read(mfd, buff, size);
#endif
			if(readbytes < 0)
			{
#ifdef WIN32
				DebugTrace("Error reading from socket: " << WSAGetLastError());
#endif
				Disconnect();
				return -1;
			}
			else if(readbytes == 0)
			{
				Disconnect();
				return 0;
			}
			else
				return readbytes;
		}
		else if( result < 0)
		{
			return -1;
		}
	}
	return 0;
}

int JSocket::Write(char* buff, int size)
{
	int size1 = size;
	while (size > 0 && state == CONNECTED)
	{
		fd_set set;
		FD_ZERO(&set);
		FD_SET(mfd, &set);
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 1000 * 100;

		int result = select(mfd+1, NULL, &set, NULL, &tv);
		if( result > 0  && FD_ISSET(mfd, &set))
		{
			int len = send(mfd, buff, size, 0);
			if (len < 0)
			{
				return -1;
			}
			size -= len;
			buff += len;
		}
		else if (result < 0)
		{
			Disconnect();
			return -1;
		}
	}
	return size1 - size;
}
