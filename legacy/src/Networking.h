void DatagramClient(char *szServer, short nPort, char * msgToSend){

	char szBuf[256];
	int nRet;

	strcpy(szBuf, msgToSend);
	nRet = sendto(theSocket, szBuf, strlen(szBuf), 0, (LPSOCKADDR)&saServer, sizeof(struct sockaddr));

	if (nRet == SOCKET_ERROR){
		PRINTERROR("sendto()");
        addMessagetoConsole(">> ERROR:sending()");
		closesocket(theSocket);
		return;
	}

	return;
}

void setupServerSocket(short nPort){
    addMessagetoConsole(">> Setup Server");
	WORD wVersionRequested = MAKEWORD(1,1);
	WSADATA wsaData;
	int nRet;

	nRet = WSAStartup(wVersionRequested, &wsaData);
	if (wsaData.wVersion != wVersionRequested){	
		fprintf(stderr,"\n Wrong version\n");
        addMessagetoConsole(">> ERROR:Wrong Version");
		return;
	}

//	SOCKET theSocket;

	theSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (theSocket == INVALID_SOCKET){
 		PRINTERROR("socket()");
        addMessagetoConsole(">> ERROR:socket()");
		return;
	}

	SOCKADDR_IN saServer;

	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = INADDR_ANY;
	saServer.sin_port = htons(nPort);

	nRet = bind(theSocket,(LPSOCKADDR)&saServer, sizeof(struct sockaddr));

	if (nRet == SOCKET_ERROR){
		PRINTERROR("bind()");
		closesocket(theSocket);
        addMessagetoConsole(">> ERROR:bind()");
		return;
	}


	int nLen;
	nLen = sizeof(SOCKADDR);
	char szBuf[256];

	nRet = gethostname(szBuf, sizeof(szBuf));
	if (nRet == SOCKET_ERROR){
		PRINTERROR("gethostname()");
		closesocket(theSocket);
        addMessagetoConsole(">> ERROR:gethostName");
		return;
	}
			

//	SOCKADDR_IN saClient;
}

void setupClientSocket(char * ip){
    addMessagetoConsole(">> Client Setup");
	WORD wVersionRequested = MAKEWORD(1,1);
	WSADATA wsaData;
	int nRet;
	short nPort;

	nPort = 4000;

	nRet = WSAStartup(wVersionRequested, &wsaData);
	if (wsaData.wVersion != wVersionRequested){	
		fprintf(stderr,"\n Wrong version\n");
        addMessagetoConsole(">> ERROR:Wrong Version");
		return;
	}

    LPHOSTENT lpHostEntry;

	lpHostEntry = gethostbyname("192.168.1.100");
    if (lpHostEntry == NULL){
        PRINTERROR("gethostbyname()");
        addMessagetoConsole(">> ERROR:gethostbyname()");
        return;
    }


	theSocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

	if (theSocket == INVALID_SOCKET){
		PRINTERROR("socket()");
        addMessagetoConsole(">> ERROR:socket()");
		return;
	}

	saServer.sin_family = AF_INET;
	saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
	saServer.sin_port = htons(nPort);
}


DWORD WINAPI readPort(LPVOID n){
	char szBuf[256];
	int nRet;
	memset(szBuf, 0, sizeof(szBuf));
	int nFromLen;
//    char* msg = (char*)n;
	while (szBuf[0] == 0){
	   nFromLen = sizeof(struct sockaddr);
	   recvfrom(theSocket,szBuf, sizeof(szBuf),0,(LPSOCKADDR)&saServer,&nFromLen);
	
	   if (nRet == SOCKET_ERROR){
	 	 PRINTERROR("recvfrom()");
		 closesocket(theSocket);
		 return 0;
	   }
	}
	addMessagetoConsole(">> Message Recieved");
	return 1;
}


DWORD readPortS(LPVOID n){
    int nLen;
	nLen = sizeof(SOCKADDR);
	char szBuf[256];
	int nRet;
    SOCKADDR_IN saClient;
	memset(szBuf, 0, sizeof(szBuf));

	nRet = recvfrom(theSocket,szBuf,sizeof(szBuf),0,(LPSOCKADDR)&saClient, &nLen);		

    if (nRet == SOCKET_ERROR){
  	  PRINTERROR("recvfrom()");
// 	  char * temp;
	  addMessagetoConsole(">> ERROR:recvfrom()");
 	  //itoa(errno, temp, 2);
	  //addMessagetoConsole(temp);
	  closesocket(theSocket);
	  return 0;
	}
	
    addMessagetoConsole(">> Message Recieved");
	return 1;
}

void StartServer(){
		//hThreads[0] = CreateThread(NULL,0,readPort,"T1",NULL,&iID[0]);
	readPortS("s");
}
