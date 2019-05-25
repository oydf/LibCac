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
#include"Thread.hpp"
#include<vector>
#include<map>
class ResponseServer
{
private:
	//正式客户队列
	std::map<SOCKET, DataClient*> _clients;
	//缓冲客户队列
	std::vector<DataClient*> _clientsBuff;
	//缓冲队列的锁
	std::mutex _mutex;
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
	//响应线程id
	int tid;
	//任务线程管理
	mThread _thread;

	void ClearClients()
	{
		for (auto iter : _clients)
		{
			delete iter.second;
		}
		_clients.clear();

		for (auto iter : _clientsBuff)
		{
			delete iter;
		}
		_clientsBuff.clear();
	}
public:
	ResponseServer(int id):tid(id),_pNetEvent(nullptr),_clients_change(true)
	{
		oldTimeStamp = Timer::getCurMill();
	}

	~ResponseServer()
	{
		Close();
	}

	void setEvent(NetEvent* event)
	{
		_pNetEvent = event;
	}

	//关闭所有套接字，从队列中遍历释放
	void Close()
	{
		_taskServer.close();
		_thread.Close();
	}
	//事件循环，处理网络事件
	void EventLoop(mThread* pThread)
	{
		while (pThread->isRun())
		{
			if (!_clientsBuff.empty())
			{   
				//从缓冲队列里取出客户数据
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients[pClient->sockfd()] = pClient;
					pClient->sid = tid;
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
			fd_set fdWrite;

			if (_clients_change)
			{
				_clients_change = false;
				FD_ZERO(&fdRead);
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
			memcpy(&fdWrite, &_fdRead, sizeof(fd_set));
			//鸡蛋，阻塞半天，注意设置超时时间
			timeval t = { 0,10 };
			int ret = select(_maxSock + 1, &fdRead, &fdWrite, nullptr, &t);
			if (ret < 0)
			{
				printf("select任务结束。\n");
				pThread->Exit();
				break;
			}
			ReadData(fdRead);
			WriteData(fdWrite);
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

	void OnClientLeave(DataClient* pClient)
	{
		if (_pNetEvent)
			_pNetEvent->OnNetLeave(pClient);
		_clients_change = true;
		delete pClient;
	}

	void WriteData(fd_set& fdWrite)
	{
#ifdef _WIN32
		for (int n = 0; n < fdWrite.fd_count; n++)
		{
			auto iter = _clients.find(fdWrite.fd_array[n]);
			if (iter != _clients.end())
			{
				if (-1 == iter->second->SendDataReal())
				{
					OnClientLeave(iter->second);
					_clients.erase(iter);
				}
			}
		}
#else
		for (auto iter = _clients.begin(); iter != _clients.end(); )
		{
			if (FD_ISSET(iter->second->sockfd(), &fdWrite))
			{
				if (-1 == iter->second->SendDataReal())
				{
					OnClientLeave(iter->second);
					auto iterOld = iter;
					iter++;
					_clients.erase(iterOld);
					continue;
				}
			}
			iter++;
		}
#endif
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

	//接收数据 处理粘包 拆分包
	int RecvData(DataClient* pClient)
	{
		//接收客户端数据
		int nLen = pClient->RecvData();
		if (nLen <= 0)
		{
			return -1;
		}
		//触发<接收到网络数据>事件
		_pNetEvent->OnNetRecv(pClient);
		//循环 判断是否有消息需要处理
		while (pClient->hasMsg())
		{
			//处理网络消息
			OnNetMsg(pClient, pClient->front_msg());
			//移除消息队列（缓冲区）最前的一条数据
			pClient->pop_front_msg();
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
		_taskServer.Start();
		_thread.Start(
			nullptr,
			[this](mThread* pThread) {
			EventLoop(pThread);
		},
			[this](mThread* pThread) {
			ClearClients();
		}
		);
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
