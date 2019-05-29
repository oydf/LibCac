/*
	������������ģʽ
	��Ϊ�����ߣ������̣߳�����ͻ�������֮��������¼�
	map���ݽṹ��������Ч��
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
	//��ʽ�ͻ�����
	std::map<SOCKET, DataClient*> _clients;
	//����ͻ�����
	std::vector<DataClient*> _clientsBuff;
	//������е���
	std::mutex _mutex;
	//�����¼�����
	NetEvent* _pNetEvent;
	//�������
	TaskServer _taskServer;
	//����fd_setԪ����
	fd_set _fdRead;
	//�ͻ��б��Ƿ��б仯
	bool _clients_change;
	//���������
	SOCKET _maxSock;
	//������ʼʱ���
	time_t oldTimeStamp;
	//��Ӧ�߳�id
	int tid;
	//�����̹߳���
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

	//�ر������׽��֣��Ӷ����б����ͷ�
	void Close()
	{
		_taskServer.close();
		_thread.Close();
	}
	//�¼�ѭ�������������¼�
	void EventLoop(mThread* pThread)
	{
		while (pThread->isRun())
		{
			if (!_clientsBuff.empty())
			{   
				//�ӻ��������ȡ���ͻ�����
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients[pClient->sockfd()] = pClient;
					pClient->sid = tid;
					printf("soc=%d����\n", pClient->sockfd());
				}
				_clientsBuff.clear();
				_clients_change = true;
			}

			//���û����Ҫ����Ŀͻ��ˣ�������
			if (_clients.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				oldTimeStamp = Timer::getCurMill();
				continue;
			}

			//��������socket������
			fd_set fdRead;
			fd_set fdWrite;

			if (_clients_change)
			{
				_clients_change = false;
				FD_ZERO(&fdRead);
				//����������socket�����뼯��
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
			//�������������죬ע�����ó�ʱʱ��
			timeval t = { 0,10 };
			int ret = select(_maxSock + 1, &fdRead, &fdWrite, nullptr, &t);
			if (ret < 0)
			{
				printf("select���������\n");
				pThread->Exit();
				break;
			}
			ReadData(fdRead);
			WriteData(fdWrite);
			CheckTime();
		}
	}

	//�������
	void CheckTime()
	{
		//��ǰʱ���
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

	//�������� ����ճ�� ��ְ�
	int RecvData(DataClient* pClient)
	{
		//���տͻ�������
		int nLen = pClient->RecvData();
		if (nLen <= 0)
		{
			return -1;
		}
		//����<���յ���������>�¼�
		_pNetEvent->OnNetRecv(pClient);
		//ѭ�� �ж��Ƿ�����Ϣ��Ҫ����
		while (pClient->hasMsg())
		{
			//����������Ϣ
			OnNetMsg(pClient, pClient->front_msg());
			//�Ƴ���Ϣ���У�����������ǰ��һ������
			pClient->pop_front_msg();
		}
		return 0;
	}

	//�û�����д���¼�
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
