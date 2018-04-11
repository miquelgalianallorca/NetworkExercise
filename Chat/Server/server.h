#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <string>

class Server {
public:
	Server()  {}
	~Server() {}

	bool Init();
	void LoopServer();
	sockaddr_in GetClient() const { return client; }
	void RemoveClient(const SOCKET &socket);
	void SendMessageToClients(const std::string &nick, const std::string &msg);

private:
	struct ServerClient {
		ServerClient(char *_nick, SOCKET _ID) :
			nick(_nick), ID(_ID) {}
		std::string nick;
		SOCKET ID;
	}; 
	
	SOCKET socketIDServer, socketIDClient;
	struct sockaddr_in client;
	std::vector<ServerClient> clients;
};

struct ConnectionData {
	ConnectionData(Server *_server, SOCKET _socket, char * _nick) :
		server(_server), socket(_socket), nick(_nick) {}
	Server *      server;
	SOCKET        socket;
	std::string   nick;
};