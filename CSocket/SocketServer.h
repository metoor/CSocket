#ifndef __SOCKET_SERVER_H__
#define __SOCKET_SERVER_H__

#include "SocketBase.h"

#define DATA_SIZE 1024
#define MAX_CONNECT 5
#define MAX_BUFFER 1024


struct RecvData
{
	SOCKET socketClient;
	int dataLen;
	char data[DATA_SIZE];
};

class SocketServer : public SocketBase
{
public:
	static SocketServer* getInstance();
	void destroyInstance();

	bool startServer(unsigned short port,bool isAsyn = false);
	void sendMessage(SOCKET socket, const char* data, int count);
	void sendMessage(const char* data, int count);

	std::function<void(const char* ip)> onStart;
	std::function<void(SOCKET socket)> onNewConnection;
	std::function<void(SOCKET socket, const char* data, int count)> onRecv;
	std::function<void(SOCKET socket)> onDisconnect;

protected:
	SocketServer();
	~SocketServer();

private:
	void clear();
	bool initServer(unsigned short port);
	void acceptClient();
	void acceptFunc();
	void startWork(bool isAsyn);	//是否是异步消息处理
	void WorkFunc();	//异步消息处理回调
	void update();
	void newClientConnected(SOCKET socket);
	void recvMessage(SOCKET socket);

private:
	static SocketServer* s_server;
	SOCKET _socketServer;
	unsigned short _serverPort;

private:
	std::list<SOCKET> _clientSockets;
	std::list<SocketMessage*> _UIMessageQueue;
	std::mutex   _UIMessageQueueMutex;

};

#endif