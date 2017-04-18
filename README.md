##CSocket
*  概述<br>
	对socket进行了一些封装，简化了使用的时候的初始化工作。接受发送数据自动异步处理，用户无需关心线程同步问题。
*  怎么使用<br>
	#####SocketServer:用户只需要添加相应的回掉函数即可。
	1. `onStart` 启动成功
	```cpp
		server->onStart = [](const char* ip) {
		cout << "server on：" << ip << " start succesed" << endl;
	};
	```	
	2. `onDisconnect` 客户端断开连接回调
	```cpp
		server->onDisconnect = [](SOCKET socket) {
		cout << socket << " disconnect" << endl;
	};
	```
	3. `onNewConnection` 新客户端连接回调
	```cpp
		server->onNewConnection = [&server](SOCKET socket) {
		cout << socket << " connected" << endl;
		server->sendMessage(socket, "Wellcome connect!",18);
	};
	```
	4. `onRecv` 接受到客户端数据
	```cpp
		server->onRecv = [&server](SOCKET socket, const char* data, int lenth) {
		cout << " recv ：" << socket << " data：" << data << endl;
	};
	};
	```
	5. `startServer(ipPort，isAsyn)` 开启服务，第一个参数是端口号，第二个参数是是否异步处	理消息
	
	#####SocketClient:用户只需要添加相应的回掉函数即可。
	1. `connectServer` 开启连接
	```cpp
		client->connectServer("127.0.0.1", 6010);
	};
	```	
	2. `onRecv` 收到服务端数据
	```cpp
		client->onRecv = [&client](const char* data,int count) {
		cout << "recv data:" << data << endl;
		client->sendMessage("Thanks you!", 12);
	};
	```
	3. `onDisconnect` 与服务端断开链接
	```cpp
		client->onDisconnect = []() {
		cout << "disconnect" << endl;
	};
	```