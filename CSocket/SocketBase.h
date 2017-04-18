/*************************************************
// Copyright (C), 2016-2020, CS&S. Co., Ltd.
// File name: 	SocketBase.h
// Author:		 Metoor
// Version: 	1.0 
// Date: 		2017/04/09
// Contact: 	caiufen@qq.com
// Description: 	create by vs2015pro
*************************************************/

#ifndef SOCKETBASE_H_  
#define SOCKETBASE_H_  
  
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

#include <list>
#include <thread>
#include <mutex>
#include <WinSock2.h>
#include "Data.h"

#pragma comment(lib, "WS2_32.lib")

#define MSG_UPDATA_INTERVAL 200  //ºÁÃë

enum MessageType
{
	DISCONNECT,
	RECEIVE,
	NEW_CONNECTION
};

class SocketMessage
{
private:
	MessageType msgType;
	Data* msgData;

public:
	SocketMessage(MessageType type, unsigned char* data, int dataLen)
	{
		msgType = type;
		msgData = new Data();
		msgData->copy(data, dataLen);
	}

	SocketMessage(MessageType type)
	{
		msgType = type;
		msgData = nullptr;
	}

	Data* getMsgData() { return msgData; }
	MessageType getMsgType() { return msgType; }

	~SocketMessage()
	{
		if (msgData)
			delete msgData;
	}
};

class SocketBase
{
public:
	SocketBase();
	~SocketBase();

	bool nonBlock(SOCKET socket);

protected:
	void closeConnect(SOCKET socket);
	bool error(SOCKET socket);

protected:
	std::mutex _mutex;

private:
	bool _bInitSuccess;
};
  
#endif // SOCKETBASE_H_ 