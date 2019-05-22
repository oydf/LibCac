/*
	主线程，负责初始化服务端工作流程、接受新的客户端连接
*/
#ifndef _MAINSERVER_H
#define _MAINSERVER_H
#include"Common.hpp"
#include"DataClient.hpp"
#include"ResponseServer.hpp"
#include"NetEvent.hpp"
#include<thread>
#include<mutex>
#include<atomic>
/*
	继承NetEvent，实现其事件接口，在主线程中只关注NetUserJion事件
*/
class MainServer : public NetEvent
{
private:
	//套接字
	SOCKET _sock;
	//消息处理对象队列，每个对象负责与相应的客户端交互，一个对象一个线程
	std::vector<ResponseServer*> _rpsServers;
	//每秒消息计时
	Timer _tTime;
protected:
	//recv计数
	std::atomic_int _recvCount;
	//收到消息计数
	std::atomic_int _msgCount;
	//客户端计数
	std::atomic_int _clientCount;
public:
	MainServer()
	{
		_sock = INVALID_SOCKET;
		_recvCount = 0;
		_msgCount = 0;
		_clientCount = 0;
	}
	virtual ~MainServer()
	{
		Close();
	}

	//1.初始化Socket
	SOCKET InitSocket()
	{
#ifdef _WIN32
		//启动Windows socket 2.x环境
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>   关闭旧连接...\n", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			printf("错误，建立socket失败...\n");
		}
		else {
			printf("建立   socket=<%d>   成功...\n", (int)_sock);
		}
		return _sock;
	}

	//2.绑定IP和端口号
	int Bind(const char* ip, unsigned short port)
	{
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		if (ip) 
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else 
		{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}
#else
		if (ip) 
		{
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else 
		{
			_sin.sin_addr.s_addr = INADDR_ANY;
		}
#endif
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret)
		{
			printf("错误,绑定网络端口   <%d>   失败...\n", port);
		}
		else 
		{
			printf("绑定网络端口   <%d>   成功...\n", port);
		}
		return ret;
	}

	//3.监听端口号
	int Listen(int n)
	{
		//listen 监听网络端口
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)
		{
			printf("socket=<%d>   错误,监听网络端口失败...\n", _sock);
		}
		else {
			printf("socket=<%d>   监听网络端口成功...\n", _sock);
		}
		return ret;
	}

	//4.接受客户端连接
	SOCKET Accept()
	{
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t *)&nAddrLen);
#endif
		if (INVALID_SOCKET == cSock)
		{
			printf("socket=<%d>错误,接受到无效客户端SOCKET...\n", (int)_sock);
		}
		else
		{
			addClientToResponseServer(new DataClient(cSock));
			//获取IP地址 inet_ntoa(clientAddr.sin_addr)
		}
		return cSock;
	}

	//将新客户端分配给客户数量最少的cellServer
	void addClientToResponseServer(DataClient* pClient)
	{
		//查找客户数量最少的消息处理对象
		auto pMinServer = _rpsServers[0];
		for (auto pCellServer : _rpsServers)
		{
			if (pMinServer->getClientCount() > pCellServer->getClientCount())
			{
				pMinServer = pCellServer;
			}
		}
		pMinServer->addClient(pClient);
		OnNetJoin(pClient);
	}

	//启动n个子线程，处理网络事件
	void Start(int nCellServer)
	{
		for (int n = 0; n < nCellServer; n++)
		{
			auto ser = new ResponseServer(_sock);
			_rpsServers.push_back(ser);
			//主线程定义的网络事件注册到子线程中
			ser->setEvent(this);
			//启动消息处理线程
			ser->Start();
		}
	}

	//关闭Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			//关闭套节字closesocket
			closesocket(_sock);
			//------------
			//清除Windows socket环境
			WSACleanup();
#else
			//关闭套节字closesocket
			close(_sock);
#endif
		}
	}

	//处理网络消息；将该事件循环设置到用户代码中，功能可扩展性更强
	bool Run()
	{
		if(isRun())
		{
			//timemsg();
			//伯克利套接字 BSD socket
			fd_set fdRead;
			FD_ZERO(&fdRead);
			//将描述符（socket）加入集合，只需要监控这一个socket
			FD_SET(_sock, &fdRead);

			timeval t = { 0,10 };
			int ret = select(_sock + 1, &fdRead, 0, 0, &t); //
			if (ret < 0)
			{
				printf("Accept Select任务结束。\n");
				Close();
				return false;
			}

			//判断描述符（socket）是否在集合中
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
				return true;
			}
			return true;
		}
		return false;
	}

	//是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//计算并输出每秒收到的网络消息
	void timemsg()
	{
		auto t1 = _tTime.getSecondsInterval();
		if (t1 >= 1.0)
		{
			printf("thread<%d>   time<%lf>   socket<%d>   clients<%d>   recv<%d>   msg<%d>\n", (int)_rpsServers.size(), t1, _sock, (int)_clientCount, (int)(_recvCount / t1), (int)(_msgCount / t1));
			_recvCount = 0;
			_msgCount = 0;
			_tTime.reset();
		} 
	}

	//只会被一个线程触发 安全
	virtual void OnNetJoin(DataClient* pClient)
	{
		_clientCount++;
		//printf("client<%d>  join\n", pClient->sockfd());
	}

	//cellServer 4 多个线程触发 不安全，如果只开启1个cellServer就是安全的
	virtual void OnNetLeave(DataClient* pClient)
	{
		_clientCount--;
		//printf("client<%d>   leave\n", pClient->sockfd());
	}

	//cellServer 4 多个线程触发 不安全，如果只开启1个cellServer就是安全的
	virtual void OnNetMsg(ResponseServer* pCellServer, DataClient* pClient, DataHeader* header)
	{
		_msgCount++;
		//printf("client<%d>   msg\n", pClient->sockfd());
	}

	virtual void OnNetRecv(DataClient* pClient)
	{
		_recvCount++;
		//printf("client<%d>  recv\n", pClient->sockfd());
	}
};
#endif // _MAINSERVER_H
