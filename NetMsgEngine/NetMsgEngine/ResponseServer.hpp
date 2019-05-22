/*
	生产者消费者模式
	作为消费者，另起线程，处理客户端连接之外的网络事件
	map数据结构提升查找效率
*/

#ifndef _RESPONSESERVER_H
#define _RESPONSESERVER_H
#include"Common.hpp"
#include"NetEvent.hpp"
#include"DataClient.hpp"
#include"SendTask.hpp"
#include<vector>
#include<map>
class ResponseServer
{
private:
	SOCKET _sock;
	//正式客户队列
	std::map<SOCKET, DataClient*> _clients;
	//缓冲客户队列
	std::vector<DataClient*> _clientsBuff;
	//缓冲队列的锁
	std::mutex _mutex;
	std::thread _thread;
	//网络事件对象
	NetEvent* _pNetEvent;
	//任务服务
	TaskServer _taskServer;
	//保存fd_set元数据
	fd_set _fdRead;
	//客户列表是否有变化
	bool _clients_change;
	//最大连接数
	SOCKET _maxSock;
	//心跳初始时间戳
	time_t oldTimeStamp;
public:
	ResponseServer(SOCKET sock = INVALID_SOCKET)
	{
		_sock = sock;
		_pNetEvent = nullptr;
		oldTimeStamp = Timer::getCurMill();
	}

	~ResponseServer()
	{
		Close();
		_sock = INVALID_SOCKET;
	}

	void setEvent(NetEvent* event)
	{
		_pNetEvent = event;
	}

	//关闭所有套接字，从队列中遍历释放
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (auto iter : _clients)
			{
				closesocket(iter.second->sockfd());
				delete iter.second;
			}
			//关闭套节字closesocket
			closesocket(_sock);
#else
			for (auto iter : _clients)
			{
				close(iter.second->sockfd());
				delete iter.second;
			}
			//关闭套节字closesocket
			close(_sock);
#endif
			_clients.clear();
		}
	}

	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//事件循环，处理网络事件
	void Run()
	{
		_clients_change = true;
		while (isRun())
		{
			if (!_clientsBuff.empty())
			{   
				//从缓冲队列里取出客户数据
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients[pClient->sockfd()] = pClient;
					printf("soc=%d加入\n", pClient->sockfd());
				}
				_clientsBuff.clear();
				_clients_change = true;
			}

			//如果没有需要处理的客户端，就跳过
			if (_clients.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				oldTimeStamp = Timer::getCurMill();
				continue;
			}

			//描述符（socket）集合
			fd_set fdRead;
			FD_ZERO(&fdRead);
			if (_clients_change)
			{
				_clients_change = false;
				//将描述符（socket）加入集合
				_maxSock = _clients.begin()->second->sockfd();
				for (auto iter : _clients)
				{
					FD_SET(iter.second->sockfd(), &fdRead);
					if (_maxSock < iter.second->sockfd())
					{
						_maxSock = iter.second->sockfd();
					}
				}
				memcpy(&_fdRead, &fdRead, sizeof(fd_set));
			}
			else 
			{
				memcpy(&fdRead, &_fdRead, sizeof(fd_set));
			}
			//鸡蛋，阻塞半天，注意设置超时时间
			timeval t = { 0,10 };
			int ret = select(_maxSock + 1, &fdRead, nullptr, nullptr, &t);
			if (ret < 0)
			{
				printf("select任务结束。\n");
				Close();
				return;
			}
			ReadData(fdRead);
			CheckTime();
		}
	}

	//心跳检测
	void CheckTime()
	{
		//当前时间戳
		auto nowTimeStamp = Timer::getCurMill();
		auto dt = nowTimeStamp - oldTimeStamp;
		oldTimeStamp = nowTimeStamp;
		for (auto iter = _clients.begin(); iter != _clients.end(); )
		{
			if (iter->second->checkHeart(dt))
			{
				if (_pNetEvent)
					_pNetEvent->OnNetLeave(iter->second);
				_clients_change = true;
				delete iter->second;
				auto iterOld = iter;
				iter++;
				_clients.erase(iterOld);
				continue;
			}
			iter++;
		}
	}

	void ReadData(fd_set& fdRead)
	{
#ifdef _WIN32
		for (int n = 0; n < fdRead.fd_count; n++)
		{
			auto iter = _clients.find(fdRead.fd_array[n]);
			if (iter != _clients.end())
			{
				if (-1 == RecvData(iter->second))
				{
					if (_pNetEvent)
						_pNetEvent->OnNetLeave(iter->second);
					_clients_change = true;
					delete iter->second;
					closesocket(iter->first);
					_clients.erase(iter);
				}
			}
			else {
				printf("error. if (iter != _clients.end())\n");
			}
		}
#else
		std::vector<CellClient*> temp;
		for (auto iter : _clients)
		{
			if (FD_ISSET(iter.second->sockfd(), &fdRead))
			{
				if (-1 == RecvData(iter.second))
				{
					if (_pNetEvent)
						_pNetEvent->OnNetLeave(iter.second);
					_clients_change = true;
					close(iter->first);
					temp.push_back(iter.second);
				}
			}
		}
		for (auto pClient : temp)
		{
			_clients.erase(pClient->sockfd());
			delete pClient;
		}
#endif
	}

	//接收数据 处理粘包 拆分包（双缓冲区）
	int RecvData(DataClient* pClient)
	{
		//接收客户端数据buff
		char* szRecv = pClient->msgBuf() + pClient->getLastPos();

		int nLen = (int)recv(pClient->sockfd(), szRecv, (RECV_BUFF_SZIE)-pClient->getLastPos(), 0);

		_pNetEvent->OnNetRecv(pClient);

		if (nLen <= 0)
		{
			//printf("客户端   <Socket=%d>   已退出，任务结束。\n", pClient->sockfd());
			return -1;
		}
		//将收取到的数据拷贝到消息缓冲区
		//memcpy(pClient->msgBuf() + pClient->getLastPos(), szRecv, nLen);
		//消息缓冲区的数据尾部位置后移
		pClient->setLastPos(pClient->getLastPos() + nLen);

		//判断消息缓冲区的数据长度大于消息头DataHeader长度
		while (pClient->getLastPos() >= sizeof(DataHeader))
		{
			//这时就可以知道当前消息的长度
			DataHeader* header = (DataHeader*)pClient->msgBuf();
			//判断消息缓冲区的数据长度大于消息长度
			if (pClient->getLastPos() >= header->dataLength)
			{
				//消息缓冲区剩余未处理数据的长度
				int nSize = pClient->getLastPos() - header->dataLength;
				//处理网络消息
				OnNetMsg(pClient, header);
				//将消息缓冲区剩余未处理数据前移
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				//消息缓冲区的数据尾部位置前移
				pClient->setLastPos(nSize);
			}
			else 
			{
				//消息缓冲区剩余数据不够一条完整消息
				break;
			}
		}
		return 0;
	}

	//用户可重写该事件
	virtual void OnNetMsg(DataClient* pClient, DataHeader* header)
	{
		_pNetEvent->OnNetMsg(this, pClient, header);
	}

	void addClient(DataClient* pClient)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_clientsBuff.push_back(pClient);
	}

	void Start()
	{
		_thread = std::thread(std::mem_fn(&ResponseServer::Run), this);
		_taskServer.Start();
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}

	void addSendTask(DataClient* pClient, DataHeader* header)
	{
		SendTask* task = new SendTask(pClient, header);
		_taskServer.addTask(task);
	}
};

#endif // _RESPONSESERVER_H
