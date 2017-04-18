#include "SocketServer.h"
#include <iostream>

using std::cout;
using std::endl;

SocketServer* SocketServer::s_server = nullptr;

SocketServer* SocketServer::getInstance()
{
	if (s_server == nullptr)
	{
		s_server = new SocketServer;
	}

	return s_server;
}

void SocketServer::destroyInstance()
{
	delete s_server;
	s_server = nullptr;
}

SocketServer::SocketServer() :
	_socketServer(0),
	onRecv(nullptr),
	onStart(nullptr),
	onNewConnection(nullptr)
{

}

SocketServer::~SocketServer()
{
	this->clear();
};

void SocketServer::clear()
{
	if (_socketServer)
	{
		_mutex.lock();
		this->closeConnect(_socketServer);
		_mutex.unlock();
	}

	for (auto msg : _UIMessageQueue)
	{
		delete msg;
	}
	_UIMessageQueue.clear();
}

bool SocketServer::startServer(unsigned short port, bool isAsyn)
{
	if (!initServer(port))
	{
		cout << "init server failed" << endl;
		return false;
	}

	startWork(isAsyn);

	return true;
}

bool SocketServer::initServer(unsigned short port)
{
	if (_socketServer != 0)
	{
		this->closeConnect(_socketServer);
	}
	_socketServer = socket(AF_INET, SOCK_STREAM, 0);
	if (error(_socketServer))
	{
		cout << "socket error!" << endl;
		_socketServer = 0;
		return false;
	}

	do
	{
		struct sockaddr_in sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));
		_serverPort = port;		// save the port

		sockAddr.sin_family = AF_INET;
		sockAddr.sin_port = htons(_serverPort);
		sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		int ret = 0;
		ret = bind(_socketServer, (const sockaddr*)&sockAddr, sizeof(sockAddr));
		if (ret < 0)
		{
			cout << "bind error!" << endl;
			break;
		}

		ret = listen(_socketServer, MAX_CONNECT);
		if (ret < 0)
		{
			cout << "listen error!" << endl;
			break;
		}
		// start 
		char hostName[256];
		gethostname(hostName, sizeof(hostName));
		struct hostent* hostInfo = gethostbyname(hostName);
		char* ip = inet_ntoa(*(struct in_addr *)*hostInfo->h_addr_list);
		this->acceptClient();

		if (onStart != nullptr)
		{
			cout << "on start" << endl;
			this->onStart(ip);
		}

		return true;

	} while (false);

	this->closeConnect(_socketServer);
	_socketServer = 0;
	return false;
}

void SocketServer::acceptClient()
{
	std::thread th(&SocketServer::acceptFunc, this);
	th.detach();
}

void SocketServer::acceptFunc()
{
	int len = sizeof(sockaddr);
	struct sockaddr_in sockAddr;
	cout << "accept connect start " << endl;
	while (s_server)
	{
		SOCKET clientSock = accept(_socketServer, (sockaddr*)&sockAddr, &len);
		if (error(clientSock))
		{
			cout << "accept error!" << endl;
		}
		else
		{
			this->newClientConnected(clientSock);
		}
	}

	cout << "accept connect end" << endl;
}


void SocketServer::startWork(bool isAsyn)
{
	if (isAsyn)
	{
		//异步处理消息
		std::thread th(&SocketServer::WorkFunc, this);
		th.detach();
		return;
	}

	WorkFunc();
}

void SocketServer::WorkFunc()
{
	cout << "WorkFunc start" << endl;
	while (s_server)
	{
		this->update();
		Sleep(MSG_UPDATA_INTERVAL);
	}
	cout << "WorkFunc quit" << endl;
}

void SocketServer::newClientConnected(SOCKET socket)
{
	cout << "new connect!" << endl;

	_clientSockets.push_back(socket);
	std::thread th(&SocketServer::recvMessage, this, socket);
	th.detach();

	if (onNewConnection)
	{
		std::lock_guard<std::mutex> lk(_UIMessageQueueMutex);
		SocketMessage * msg = new SocketMessage(NEW_CONNECTION, (unsigned char*)&socket, sizeof(SOCKET));
		_UIMessageQueue.push_back(msg);
	}
}

void SocketServer::recvMessage(SOCKET socket)
{
	char buff[MAX_BUFFER];
	int ret = 0;

	while (true)
	{
		ret = recv(socket, buff, sizeof(buff), 0);
		if (ret < 0)
		{
			cout << "recv" << socket << "error!" << endl;
			break;
		}
		else
		{
			if (ret > 0 && onRecv != nullptr)
			{
				std::lock_guard<std::mutex> lk(_UIMessageQueueMutex);
				RecvData recvData;
				recvData.socketClient = socket;
				memcpy(recvData.data, buff, ret);
				recvData.dataLen = ret;
				SocketMessage * msg = new SocketMessage(RECEIVE, (unsigned char*)&recvData, sizeof(RecvData));
				_UIMessageQueue.push_back(msg);
			}
		}
	}

	_mutex.lock();
	this->closeConnect(socket);
	if (onDisconnect != nullptr)
	{
		std::lock_guard<std::mutex> lk(_UIMessageQueueMutex);
		SocketMessage * msg = new SocketMessage(DISCONNECT, (unsigned char*)&socket, sizeof(SOCKET));
		_UIMessageQueue.push_back(msg);
	}
	_mutex.unlock();
}

void SocketServer::sendMessage(SOCKET socket, const char* data, int count)
{
	for (auto& sock : _clientSockets)
	{
		if (sock == socket)
		{
			int ret = send(socket, data, count, 0);
			if (ret < 0)
			{
				cout << "send error!" << endl;
			}
			break;
		}
	}
}

void SocketServer::sendMessage(const char* data, int count)
{
	for (auto& socket : _clientSockets)
	{
		int ret = send(socket, data, count, 0);
		if (ret < 0)
		{
			cout << "send error!" << endl;
		}
	}
}

void SocketServer::update()
{
	if (_UIMessageQueue.size() == 0)
	{
		return;
	}

	_UIMessageQueueMutex.lock();

	if (_UIMessageQueue.size() == 0)
	{
		_UIMessageQueueMutex.unlock();
		return;
	}

	SocketMessage *msg = *(_UIMessageQueue.begin());
	_UIMessageQueue.pop_front();

	switch (msg->getMsgType())
	{
	case NEW_CONNECTION:
		if (onNewConnection)
		{
			this->onNewConnection(*(SOCKET*)msg->getMsgData()->getBytes());
		}
		break;
	case DISCONNECT:
		if (onDisconnect)
		{
			this->onDisconnect(*(SOCKET*)msg->getMsgData()->getBytes());
		}
		break;
	case RECEIVE:
		if (onRecv)
		{
			RecvData* recvData = (RecvData*)msg->getMsgData()->getBytes();
			this->onRecv(recvData->socketClient, (const char*)recvData->data, recvData->dataLen);
		}
		break;
	default:
		break;
	}

	delete msg;
	_UIMessageQueueMutex.unlock();
}