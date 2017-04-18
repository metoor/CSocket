/*************************************************
// Copyright (C), 2016-2020, CS&S. Co., Ltd.
// File name: 	SocketClient.cpp
// Author:		 Metoor
// Version: 	1.0
// Date: 		2017/04/09
// Contact: 	caiufen@qq.com
// Description: 	create by vs2015pro
*************************************************/

#include "SocketClient.h"
#include <iostream>

using std::cout;
using std::endl;


SocketClient* SocketClient::create()
{
	SocketClient* client = new SocketClient;
	return client;
}

void SocketClient::destroy()
{
	delete this;
}

SocketClient::SocketClient(void) :
	onRecv(nullptr),
	_socektClient(0)
{
}

SocketClient::~SocketClient(void)
{
	this->clear();
}

void SocketClient::clear()
{
	if (_socektClient != 0)
	{
		_mutex.lock();
		this->closeConnect(_socektClient);
		_mutex.unlock();
	}

	for (auto msg : _UIMessageQueue)
	{
		delete msg;
	}
	_UIMessageQueue.clear();
}

bool SocketClient::initClient()
{
	this->clear();

	_socektClient = socket(AF_INET, SOCK_STREAM, 0);
	if (error(_socketServer))
	{
		cout << "init client error!" << endl;
		_socektClient = 0;
		return false;
	}

	return true;
}

bool SocketClient::connectServer(const char* serverIP, unsigned short port, bool isAsyn)
{
	if (!this->initClient())
	{
		return false;
	}

	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(struct sockaddr_in));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr(serverIP);

	int ret = 0;
	ret = connect(_socektClient, (struct sockaddr*)&serverAddr, sizeof(struct sockaddr));
	if (ret < 0)
	{
		_socektClient = 0;
		return false;
	}

	std::thread recvThread(&SocketClient::recvMessage, this);
	recvThread.detach();
	
	startWork(isAsyn);
	
	return true;
}

void SocketClient::recvMessage()
{
	char recvBuf[MAX_BUFFER];
	int ret = 0;
	cout << "recvMessage start" << endl;
	while (true)
	{
		ret = recv(_socektClient, recvBuf, sizeof(recvBuf), 0);
		if (ret < 0)
		{
			cout << "recv error!" << endl;
			break;
		}
		if (ret > 0 && onRecv != nullptr)
		{
			std::lock_guard<std::mutex> lk(_UIMessageQueueMutex);
			SocketMessage * msg = new SocketMessage(RECEIVE, (unsigned char*)recvBuf, ret);
			_UIMessageQueue.push_back(msg);
		}
	}
	_mutex.lock();
	this->closeConnect(_socektClient);
	if (onDisconnect != nullptr)
	{
		std::lock_guard<std::mutex> lk(_UIMessageQueueMutex);
		SocketMessage * msg = new SocketMessage(DISCONNECT);
		_UIMessageQueue.push_back(msg);
	}
	_socektClient = 0;
	_mutex.unlock();
	cout << "recvMessage end" << endl;
}

void SocketClient::sendMessage(const char* data, int count)
{
	if (_socektClient != 0)
	{
		int ret = send(_socektClient, data, count, 0);
		if (ret < 0)
		{
			cout << "send error!" << endl;
		}
	}
}

void SocketClient::startWork(bool isAsyn)
{
	if (isAsyn)
	{
		std::thread th(&SocketClient::WorkFunc, this);
		th.detach();
		return;
	}

	WorkFunc();
}

void SocketClient::WorkFunc()
{
	cout << "WorkFunc start" << endl;
	while (!this->update())
	{
		Sleep(MSG_UPDATA_INTERVAL);
	}
	cout << "WorkFunc end" << endl;
}

bool SocketClient::update()
{
	bool isQuit = false;

	if (_UIMessageQueue.size() == 0)
	{
		return isQuit;
	}

	_UIMessageQueueMutex.lock();

	if (_UIMessageQueue.size() == 0)
	{
		_UIMessageQueueMutex.unlock();
		return isQuit;
	}

	SocketMessage *msg = *(_UIMessageQueue.begin());
	_UIMessageQueue.pop_front();

	switch (msg->getMsgType())
	{
	case DISCONNECT:
		if (onDisconnect)
		{
			this->onDisconnect();
			isQuit = true;
		}
		break;
	case RECEIVE:
		if (onRecv)
		{
			this->onRecv((const char*)msg->getMsgData()->getBytes(), msg->getMsgData()->getSize());
		}
		break;
	default:
		break;
	}

	delete msg;
	_UIMessageQueueMutex.unlock();

	return isQuit;
}