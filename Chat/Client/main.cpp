#include <stdio.h>
#include <iostream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>

#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>

using namespace std;

#define SERVER_PORT "12345"
#define BUF_SIZE 4096

// CLIENT ============================================================================

void * listenToServer(void* argument) {
	SOCKET * fdSocket = (SOCKET*)argument;
    bool isListening = true;
	while (isListening) {
        char buf[BUF_SIZE];
        // Recv: Receive data from connection
        int totalRecv = 0;
        do {
            int rec = recv(*fdSocket, buf + totalRecv, BUF_SIZE, 0);
            if (rec == SOCKET_ERROR) {
                cout << "Server error: connection lost." << endl;
                isListening = false;
                break;
            }
            else totalRecv += rec;
        } while (buf[totalRecv - 1] != '\0');

        if (!isListening) break;

        // Server message
        cout << buf << endl;
	}

    closesocket(*fdSocket);
	delete fdSocket;
	return nullptr;
}

int main(int argc, char *argv[])
{
	// Params (IP)
	if (argc < 2) return -1;

	// Start WinSock
	WSADATA wsaData;
	SOCKET sockfd;
	char buf[BUF_SIZE];
	memset(&buf, 0, sizeof(buf));
	int wsaerr = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsaerr != 0)
	{
		printf("Could not initialize Winsock.\n");
		return -1;
	}
	// Set IP info
	struct addrinfo  hints;   // hints de la direccion buscada
	struct addrinfo* servInfo;// contendra la lista de direccciones encontradas
	struct addrinfo* srvaddr;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Busca las addrinfo del host al que nos queremos conectar
	int error = getaddrinfo(argv[1], SERVER_PORT, &hints, &servInfo);
	if (error != 0) {
		printf("getaddrinfo failed");
		WSACleanup();
		return -1;
	}
	
	// Socket: Open connection socket
	for (srvaddr = servInfo; (srvaddr != NULL); srvaddr = srvaddr->ai_next) {
		sockfd = socket(srvaddr->ai_family, srvaddr->ai_socktype, srvaddr->ai_protocol);
		if (sockfd != INVALID_SOCKET) {
			int iResult = connect(sockfd, srvaddr->ai_addr, (int)srvaddr->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				closesocket(sockfd);
				sockfd = INVALID_SOCKET;
			}
			else break;
		}
	}
	freeaddrinfo(servInfo);
	
	if (sockfd != INVALID_SOCKET) {
		// Enter username
		printf("Enter username: ");
		gets_s(buf, _countof(buf));
		
		int length = strlen(buf) + 1;
		int numBytesSend, totalSend = 0;
		do {
			numBytesSend = send(sockfd, buf, length - totalSend, 0);
			totalSend += numBytesSend;
		} while (totalSend < length);

		//Connect user
		pthread_t listenThread;
		pthread_create(&listenThread, nullptr, listenToServer, &sockfd);

		bool isConnected = true;
		while (isConnected) {
			// Send: Send data to connection
			gets_s(buf, _countof(buf));
			if (strcmp(buf, "q") == 0) {
				printf("Quitting.\n");
				isConnected = false;
			}
			int length = strlen(buf) + 1;
			int numBytesSend, totalSend = 0;
			do {
				numBytesSend = send(sockfd, buf, length - totalSend, 0);
				totalSend += numBytesSend;
			} while (totalSend < length);
		}
	}

	// Close: End connection
	closesocket(sockfd);
	WSACleanup();
	return 0;
}