#include "server.h"

// SERVER
int main(int argc, char *argv[])
{
	Server server;
	if (server.Init())
		server.LoopServer();

	return 0;
}
