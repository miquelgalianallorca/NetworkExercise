#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>

class Server {
public:
	Server()  {}
	~Server() {}

	bool Init();
	void LoopServer();
	sockaddr_in GetClient() const { return client; }

private:
	SOCKET socketIDServer, socketIDClient;
	struct sockaddr_in client;

	struct ServerClient {
		ServerClient(char *_nick, SOCKET _ID) :
			nick(_nick), ID(_ID) {}
		char * nick;
		SOCKET ID;
	};

	std::vector<ServerClient> clients;
};

struct ConnectionData {
	ConnectionData(Server *_server, SOCKET _socket) :
		server(_server), socket(_socket) {}
	Server *server;
	SOCKET socket;
};