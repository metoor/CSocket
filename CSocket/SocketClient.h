/*************************************************
// Copyright (C), 2016-2020, CS&S. Co., Ltd.
// File name: 	SocketClient.h
// Author:		 Metoor
// Version: 	1.0 
// Date: 		2017/04/09
// Contact: 	caiufen@qq.com
// Description: 	create by vs2015pro
*************************************************/

#ifndef SOCKETCLIENT_H_  
#define SOCKETCLIENT_H_  
  
#include "SocketBase.h"

#define MAX_BUFFER 1024

class SocketClient : public SocketBase
{
public:
	static SocketClient* create();
	void destroy();

	bool connectServer(const char* serverIP, unsigned short port, bool isAsyn = true);
	void sendMessage(const char* data, int count);

	std::function<void(const char* data, int count)> onRecv;
	std::function<void()> onDisconnect;

protected:
	SocketClient(void);
	~SocketClient(void);

private:
	bool initClient();
	void startWork(bool isAsyn = false);
	bool update();
	void WorkFunc();
	void recvMessage();
	void clear();

private:
	SOCKET _socketServer;
	SOCKET _socektClient;
	std::list<SocketMessage*> _UIMessageQueue;
	std::mutex   _UIMessageQueueMutex;
};

#endif // SOCKETCLIENT_H_ 