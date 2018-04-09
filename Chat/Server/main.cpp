#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>

#define SERVER_PORT 12345
#define BUF_SIZE 4096
#define QUEUE_SIZE 10

// SERVER
int main(int argc, char *argv[])
{
	// Start WinSock
	WSADATA wsaData;
	SOCKET sockId, socketCliId;
	int on = 1;
	struct sockaddr_in addr;
	struct sockaddr_in client;
	char buf[BUF_SIZE];
	int wsaerr = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsaerr != 0) {
		printf("Could not initialize Winsock.\n");
		return -1;
	}
	// Set IP info
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET; // Aquí NO se usa htons.
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(SERVER_PORT);

	// Socket: Open connection socket
	sockId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockId < 0) {
		printf("Error creating socket\n");
		WSACleanup();
		return -1;
	}
	setsockopt(sockId, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	
	// Bind: Assign address to socket
	if (bind(sockId, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		printf("Error binding port\n");
		WSACleanup();
		return -1;
	}

	// Listen: Available to receive client connections
	if (listen(sockId, QUEUE_SIZE) < 0) {
		printf("listen failed\n");
		WSACleanup();
		return -1;
	}

	// Loop
	while (1) {
		// Accept: Block until received connection
		socklen_t sock_len = sizeof(client);
		memset(&client, 0, sizeof(client));
		socketCliId = accept(sockId, (struct sockaddr *)&client, &sock_len);
		if (socketCliId == INVALID_SOCKET)
			printf("Can't accept client:%d\n", WSAGetLastError());
		else {
			bool isClientOnline = true;
			while (isClientOnline) {
				// Recv: Receive data from connection
				int totalRecv = 0;
				do {
					int rec = recv(socketCliId, buf + totalRecv, BUF_SIZE, 0);
					if (rec == SOCKET_ERROR) {
						printf("Lost connection to client.\n");
						isClientOnline = false;
					}
					totalRecv += rec;
					printf("rec:%d | totalRecv: %d\n", rec, totalRecv);
				} while (buf[totalRecv - 1] != '\0');
				
				// Client message
				if (strcmp(buf, "quit") == 0) {
					printf("Client quitted.\n");
					isClientOnline = false;
				}
				else {
					char ipClient[INET_ADDRSTRLEN];
					inet_ntop(AF_INET, &client.sin_addr, ipClient, sizeof(ipClient));
					printf("Client:%s Port:%d\n", ipClient, ntohs(client.sin_port));
					printf("Message Length: %d\n", totalRecv);
					printf("Message: %s\n", buf);
				}
			}
			// Close: End connection
			closesocket(socketCliId);
		}
	}
	closesocket(sockId);
	WSACleanup();
	return 0;
}