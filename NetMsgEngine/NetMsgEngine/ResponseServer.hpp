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
#include<vector>
#include<map>
class ResponseServer
{
private:
	SOCKET _sock;
	//��ʽ�ͻ�����
	std::map<SOCKET, DataClient*> _clients;
	//����ͻ�����
	std::vector<DataClient*> _clientsBuff;
	//������е���
	std::mutex _mutex;
	std::thread _thread;
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

	//�ر������׽��֣��Ӷ����б����ͷ�
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
			//�ر��׽���closesocket
			closesocket(_sock);
#else
			for (auto iter : _clients)
			{
				close(iter.second->sockfd());
				delete iter.second;
			}
			//�ر��׽���closesocket
			close(_sock);
#endif
			_clients.clear();
		}
	}

	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//�¼�ѭ�������������¼�
	void Run()
	{
		_clients_change = true;
		while (isRun())
		{
			if (!_clientsBuff.empty())
			{   
				//�ӻ��������ȡ���ͻ�����
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients[pClient->sockfd()] = pClient;
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
			FD_ZERO(&fdRead);
			if (_clients_change)
			{
				_clients_change = false;
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
			//�������������죬ע�����ó�ʱʱ��
			timeval t = { 0,10 };
			int ret = select(_maxSock + 1, &fdRead, nullptr, nullptr, &t);
			if (ret < 0)
			{
				printf("select���������\n");
				Close();
				return;
			}
			ReadData(fdRead);
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

	//�������� ����ճ�� ��ְ���˫��������
	int RecvData(DataClient* pClient)
	{
		//���տͻ�������buff
		char* szRecv = pClient->msgBuf() + pClient->getLastPos();

		int nLen = (int)recv(pClient->sockfd(), szRecv, (RECV_BUFF_SZIE)-pClient->getLastPos(), 0);

		_pNetEvent->OnNetRecv(pClient);

		if (nLen <= 0)
		{
			//printf("�ͻ���   <Socket=%d>   ���˳������������\n", pClient->sockfd());
			return -1;
		}
		//����ȡ�������ݿ�������Ϣ������
		//memcpy(pClient->msgBuf() + pClient->getLastPos(), szRecv, nLen);
		//��Ϣ������������β��λ�ú���
		pClient->setLastPos(pClient->getLastPos() + nLen);

		//�ж���Ϣ�����������ݳ��ȴ�����ϢͷDataHeader����
		while (pClient->getLastPos() >= sizeof(DataHeader))
		{
			//��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
			DataHeader* header = (DataHeader*)pClient->msgBuf();
			//�ж���Ϣ�����������ݳ��ȴ�����Ϣ����
			if (pClient->getLastPos() >= header->dataLength)
			{
				//��Ϣ������ʣ��δ�������ݵĳ���
				int nSize = pClient->getLastPos() - header->dataLength;
				//����������Ϣ
				OnNetMsg(pClient, header);
				//����Ϣ������ʣ��δ��������ǰ��
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				//��Ϣ������������β��λ��ǰ��
				pClient->setLastPos(nSize);
			}
			else 
			{
				//��Ϣ������ʣ�����ݲ���һ��������Ϣ
				break;
			}
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
