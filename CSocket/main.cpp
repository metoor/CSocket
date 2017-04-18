#include <iostream>
#include <iostream>
#include "SocketServer.h"

using namespace std;

int main()
{
	auto server = SocketServer::getInstance();

	server->onDisconnect = [](SOCKET socket) {
		cout << socket << " disconnect" << endl;
	};

	server->onNewConnection = [&server](SOCKET socket) {
		cout << socket << " connected" << endl;
		server->sendMessage(socket, "Wellcome connect!",18);
	};

	server->onStart = [](const char* ip) {
		cout << "server on£º" << ip << " start succesed" << endl;
	};

	server->onRecv = [&server](SOCKET socket, const char* data, int lenth) {
		cout << " recv £º" << socket << " data£º" << data << endl;
	};

	server->startServer(6010);

	getchar();
	return 0;
}