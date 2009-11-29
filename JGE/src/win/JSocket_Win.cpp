#pragma comment(lib,"WSOCK32.LIB")
#include <stdio.h>
#include <conio.h>
#include <winsock.h>
#include <errno.h>
#include <winsock.h>

#include "../../include/JSocket.h"
JSocket * JSocket::mInstance = NULL;

#define SERVER_PORT 20666
unsigned char ADRESSE_IP_SERVEUR [4] = {127,0,0,1};


int JSocket::connected = 0;

void JSocket::init(){
	//TODO ?
}

JSocket::JSocket(){
  init();
}

JSocket::~JSocket(){
 //TODO ?
}


void JSocket::readWrite(int val){
  char data[1024];

  	fd_set set;
    FD_ZERO(&set);
	  FD_SET(val, &set);
    struct timeval tv;
   int result = select(0, &set, NULL, NULL, &tv);
		if( result< 0)
		{
      printf("Socket %d closed\n", val);
		  closesocket(val);
      connected= 0;
			printf("select error\n");
			return;
		}
    
    if (result > 0){
      OutputDebugString("Receiving!\n");
      int readbytes = recv(val, data, sizeof(data),0);
	    if(readbytes < 0)
	    {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK){
		      printf("Socket %d closed\n", val);
		      closesocket(val);
          connected= 0;
        }
	    }
	    else
	    {
        OutputDebugString("received Data!!!\n");
		    for (int i = 0; i < readbytes; i++){
		      received_data.push(data[i]);
          OutputDebugString("getting byte\n");
		    }
	    }
    }

  //Write
  int size = 0;
  while(!tosend_data.empty()){
    size++;
    if (size > sizeof(data)){
      send(val,data,sizeof(data),0);
      size = 0;
    }
    data[size-1] = tosend_data.front();
    tosend_data.pop();
  }
  if (size) {
    send(val,data,size-1,0);
    OutputDebugString("sending Data\n");
  }
}

/* Start a client */
int JSocket::start_client(const char *szIpAddr){
  SOCKET Desc_Socket_Cliente;
  SOCKADDR_IN Adresse_Socket_Serveur;
  WORD wVersionRequested;
  WSADATA wsaData;

  struct hostent *hostentptr;

  wVersionRequested=MAKEWORD(1,1);
  WSAStartup(wVersionRequested,&wsaData);

  Desc_Socket_Cliente=socket(AF_INET,SOCK_STREAM,0);
  unsigned int addr_dest = inet_addr(szIpAddr); 
  hostentptr=gethostbyaddr((char*) &addr_dest,4,AF_INET);

  ZeroMemory(&Adresse_Socket_Serveur,sizeof(Adresse_Socket_Serveur));

  Adresse_Socket_Serveur.sin_family=(*hostentptr).h_addrtype;
  Adresse_Socket_Serveur.sin_port=htons(SERVER_PORT);
  Adresse_Socket_Serveur.sin_addr=*((struct in_addr*)(*hostentptr).h_addr);

  int result = connect(
    Desc_Socket_Cliente,
    (const struct sockaddr*)&Adresse_Socket_Serveur,
    sizeof(Adresse_Socket_Serveur));

OutputDebugString("client state 0\n");

  if (result != 0){
    connected = 0;
    return 0;
  }

OutputDebugString("client state 1\n");


  connected = 1;

while(1){
  readWrite(Desc_Socket_Cliente);
/*  Nb_Caracteres_Recus=recv(Desc_Socket_Cliente,Message_Recu,sizeof(Message_Recu),0);
  if(Nb_Caracteres_Recus<=0){
    //continuer=FALSE;
  }else{
    strcpy(message,Message_Recu);
    len=strlen(message);
  }
  */
}
#
closesocket(Desc_Socket_Cliente);
WSACleanup();
return 0;
}

/* Start a server */
int JSocket::start_server(const char *szIpAddr){
  OutputDebugString("server state 0\n");
  connected= 0;
  int Code_Retour;
  SOCKET Desc_Socket_Connection;
  SOCKADDR_IN Adresse_Socket_Connection;
  WORD wVersionRequested;
  WSADATA wsaData;

  wVersionRequested=MAKEWORD(1,1);

  Code_Retour=WSAStartup(wVersionRequested,&wsaData);

  if(Code_Retour!=0){
    perror("WSAStartup\t");
    _getch();
    WSACleanup();
    return Code_Retour;
  }
  OutputDebugString("server state 1\n");
/*    printf("la version supportee est : %d.%d\n",
LOBYTE(wsaData.wHighVersion),
HIBYTE(wsaData.wHighVersion)
);*/

  Desc_Socket_Connection=socket( AF_INET, SOCK_STREAM,0);

//printf("valeur de la socket = %d\n",Desc_Socket_Connection);

  ZeroMemory(&Adresse_Socket_Connection,sizeof(Adresse_Socket_Connection));
  Adresse_Socket_Connection.sin_family=AF_INET;
  Adresse_Socket_Connection.sin_port=htons(SERVER_PORT);

  Code_Retour=bind(Desc_Socket_Connection,
    (struct sockaddr*)&Adresse_Socket_Connection,
    sizeof(Adresse_Socket_Connection));



  if(Code_Retour!=0){
    perror("bind\t");
    _getch();
    closesocket(Desc_Socket_Connection);
    WSACleanup();
    return Code_Retour;
  }
  OutputDebugString("server state 3\n");

  Code_Retour=listen(Desc_Socket_Connection,1);
  if(Code_Retour!=0){
    perror("listen\n");
    WSACleanup();
    return Code_Retour;
  }
  OutputDebugString("server state 4\n");
  printf("serveur en attente d'une connection\n\n");
  printf("***************arret du serveur par<CTRL><C>**************\n\n");
  connected = 1;
  while(1){
    SOCKET * pt_Nouveau_Socket_Serveur;
    SOCKADDR_IN Adresse_Socket_Cliente;
    int Longueur_Adresse;

    pt_Nouveau_Socket_Serveur = new SOCKET;

    Longueur_Adresse = sizeof(Adresse_Socket_Cliente);

    int val=accept(
      Desc_Socket_Connection,
      (struct sockaddr*)&Adresse_Socket_Cliente,
      &Longueur_Adresse);
    printf("connection accepte depuis le port client %d\n", ntohs(Adresse_Socket_Cliente.sin_port));
      OutputDebugString("connection accepte depuis le port client\n");


    while(1)
    {
	    readWrite(val);
    }
    closesocket(*pt_Nouveau_Socket_Serveur);
    return 0;
  }
}





