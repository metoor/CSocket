/*************************************************
// Copyright (C), 2016-2020, CS&S. Co., Ltd.
// File name: 	SocketBase.cpp
// Author:		 Metoor
// Version: 	1.0 
// Date: 		2017/04/09
// Contact: 	caiufen@qq.com
// Description: 	create by vs2015pro
*************************************************/

#include "SocketBase.h"

SocketBase::SocketBase()
{
	_bInitSuccess = false;
	WORD wVersionRequested;
	wVersionRequested = MAKEWORD(2, 0);
	WSADATA wsaData;
	int nRet = WSAStartup(wVersionRequested, &wsaData);
	if (nRet != 0) 
	{
		fprintf(stderr, "Initilize Error!\n");
		return;
	}
	_bInitSuccess = true;
}

SocketBase::~SocketBase()
{
	WSACleanup();
}

void SocketBase::closeConnect(SOCKET socket)
{
	closesocket(socket);
}

bool SocketBase::error(SOCKET socket)
{
	return socket == SOCKET_ERROR;
}

bool SocketBase::nonBlock(SOCKET socket)
{
	u_long ulOn;
	ulOn = 1;
	if (ioctlsocket(socket, FIONBIO, &ulOn) == SOCKET_ERROR)
	{
		return false;
	}

	return true;
}