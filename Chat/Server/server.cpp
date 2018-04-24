#include "server.h"

#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#include <algorithm>

#define SERVER_PORT 12345
#define BUF_SIZE 4096
#define QUEUE_SIZE 10

using std::cout;
using std::endl;

void * listenToClient(void * argument) {
	ConnectionData *connectionData = (ConnectionData*) argument;
	bool isClientListening = true;
	while (isClientListening) {
		char buf[BUF_SIZE];
		// Recv: Receive data from connection
		int totalRecv = 0;
		do {
			int rec = recv(connectionData->socket, buf + totalRecv, BUF_SIZE, 0);
			if (rec == SOCKET_ERROR) {
				cout << connectionData->nick << " left the room." << endl;
				isClientListening = false;
				break;
			}
			else {
				totalRecv += rec;
				//printf("rec:%d | totalRecv: %d\n", rec, totalRecv);
			}
		} while (buf[totalRecv - 1] != '\0');

		if (!isClientListening) break;

		// Client Message
		if (strcmp(buf, "q") == 0) {
			cout << connectionData->nick << " left the room." << endl;
			connectionData->server->SendMessageToClients(buf);
			isClientListening = false;
			break;
		}
		else {
			char ipClient[INET_ADDRSTRLEN];
			sockaddr_in client = connectionData->server->GetClient();
			inet_ntop(AF_INET, &client.sin_addr, ipClient,
                sizeof(ipClient));
			//printf("Client:%s Port:%d\n", ipClient, ntohs(client.sin_port));
			//printf("Message Length: %d\n", totalRecv);
			//printf("Message: %s\n", buf);
			cout << connectionData->nick << ":> " << buf << endl;
            std::string msg = connectionData->nick + std::string(":> ") + buf;
			connectionData->server->SendMessageToClients(msg);
		}
	}
	
	connectionData->server->RemoveClient(connectionData->socket);	
	closesocket(connectionData->socket);
	delete connectionData;
	return nullptr;
}

bool Server::Init() {
	WSADATA wsaData;
	int on = 1;
	struct sockaddr_in addr;
	int wsaerr = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsaerr != 0) {
		printf("Could not initialize Winsock.\n");
		return false;
	}
	// Set IP info
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET; // Aquí NO se usa htons.
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(SERVER_PORT);

	// Socket: Open connection socket
	socketIDServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socketIDServer < 0) {
		printf("Error creating socket\n");
		WSACleanup();
		return false;
	}
	setsockopt(socketIDServer, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));

	// Bind: Assign address to socket
	if (bind(socketIDServer, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		printf("Error binding port\n");
		WSACleanup();
		return false;
	}
	// Listen: Available to receive client connections
	if (listen(socketIDServer, QUEUE_SIZE) < 0) {
		printf("listen failed\n");
		WSACleanup();
		return false;
	}

	return true;
}

void Server::LoopServer() {
	char buf[BUF_SIZE];
	while (true) {
		// Accept: Block until received connection
		socklen_t sock_len = sizeof(client);
		memset(&client, 0, sizeof(client));
		socketIDClient = accept(socketIDServer, (struct sockaddr *)&client, &sock_len);
		if (socketIDClient == INVALID_SOCKET) {
			printf("Can't accept client:%d\n", WSAGetLastError());
		}
		else {
			// Client entered
			int totalRecv = 0;
            bool couldConnect = false;
            bool seguir = true;
            while (seguir) {
                int rec = recv(socketIDClient, buf + totalRecv, BUF_SIZE, 0);
                if (rec == SOCKET_ERROR) {
                    printf("Lost connection to client.\n");
                    seguir = false;
                }
                else totalRecv += rec;
                if (buf[totalRecv - 1] == '\0') {
                    seguir = false;
                    couldConnect = true;
                }
            }
            // Client entered correctly
            if (couldConnect) {
                // Tell everyone
                std::string msg = buf + std::string(" entered the room.");
                cout << msg << endl;
                SendMessageToClients(msg);

                // Add client
                pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
                //------------------------------------------------------------------------
                pthread_mutex_lock(&mutex);
                clients.push_back(ServerClient(buf, socketIDClient));
                pthread_t listenThread;
                ConnectionData *connectionData = new ConnectionData(this, socketIDClient, buf);
                pthread_create(&listenThread, nullptr, listenToClient, connectionData);
                pthread_mutex_unlock(&mutex);
                //------------------------------------------------------------------------
            }
        }
	}
	closesocket(socketIDServer);
	WSACleanup();
}

void Server::RemoveClient(const SOCKET &socket) {
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	//---------------------------------------------------------------
	pthread_mutex_lock(&mutex);
	clients.erase(
		std::remove_if(clients.begin(), clients.end(),
			[socket](const ServerClient &s) { return s.ID == socket; }
		),
		clients.end()
	);
	pthread_mutex_unlock(&mutex);
	//---------------------------------------------------------------
}

void Server::SendMessageToClients(const std::string &msg) {
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    //---------------------------------------------------------------
    pthread_mutex_lock(&mutex);
    
    // Send message
    for (auto client : clients) {
        if (client.ID != INVALID_SOCKET){
            int length = strlen(msg.data()) + 1;
            int numBytesSend, totalSend = 0;
            do {
                numBytesSend = send(client.ID, msg.data(),
                    length - totalSend, 0);
                totalSend += numBytesSend;
            } while (totalSend < length);
        }
    }

    pthread_mutex_unlock(&mutex);
    //---------------------------------------------------------------
}