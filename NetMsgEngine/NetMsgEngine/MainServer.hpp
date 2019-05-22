/*
	���̣߳������ʼ������˹������̡������µĿͻ�������
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
	�̳�NetEvent��ʵ�����¼��ӿڣ������߳���ֻ��עNetUserJion�¼�
*/
class MainServer : public NetEvent
{
private:
	//�׽���
	SOCKET _sock;
	//��Ϣ���������У�ÿ������������Ӧ�Ŀͻ��˽�����һ������һ���߳�
	std::vector<ResponseServer*> _rpsServers;
	//ÿ����Ϣ��ʱ
	Timer _tTime;
protected:
	//recv����
	std::atomic_int _recvCount;
	//�յ���Ϣ����
	std::atomic_int _msgCount;
	//�ͻ��˼���
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

	//1.��ʼ��Socket
	SOCKET InitSocket()
	{
#ifdef _WIN32
		//����Windows socket 2.x����
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>   �رվ�����...\n", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			printf("���󣬽���socketʧ��...\n");
		}
		else {
			printf("����   socket=<%d>   �ɹ�...\n", (int)_sock);
		}
		return _sock;
	}

	//2.��IP�Ͷ˿ں�
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
			printf("����,������˿�   <%d>   ʧ��...\n", port);
		}
		else 
		{
			printf("������˿�   <%d>   �ɹ�...\n", port);
		}
		return ret;
	}

	//3.�����˿ں�
	int Listen(int n)
	{
		//listen ��������˿�
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)
		{
			printf("socket=<%d>   ����,��������˿�ʧ��...\n", _sock);
		}
		else {
			printf("socket=<%d>   ��������˿ڳɹ�...\n", _sock);
		}
		return ret;
	}

	//4.���ܿͻ�������
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
			printf("socket=<%d>����,���ܵ���Ч�ͻ���SOCKET...\n", (int)_sock);
		}
		else
		{
			addClientToResponseServer(new DataClient(cSock));
			//��ȡIP��ַ inet_ntoa(clientAddr.sin_addr)
		}
		return cSock;
	}

	//���¿ͻ��˷�����ͻ��������ٵ�cellServer
	void addClientToResponseServer(DataClient* pClient)
	{
		//���ҿͻ��������ٵ���Ϣ�������
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

	//����n�����̣߳����������¼�
	void Start(int nCellServer)
	{
		for (int n = 0; n < nCellServer; n++)
		{
			auto ser = new ResponseServer(_sock);
			_rpsServers.push_back(ser);
			//���̶߳���������¼�ע�ᵽ���߳���
			ser->setEvent(this);
			//������Ϣ�����߳�
			ser->Start();
		}
	}

	//�ر�Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			//�ر��׽���closesocket
			closesocket(_sock);
			//------------
			//���Windows socket����
			WSACleanup();
#else
			//�ر��׽���closesocket
			close(_sock);
#endif
		}
	}

	//����������Ϣ�������¼�ѭ�����õ��û������У����ܿ���չ�Ը�ǿ
	bool Run()
	{
		if(isRun())
		{
			//timemsg();
			//�������׽��� BSD socket
			fd_set fdRead;
			FD_ZERO(&fdRead);
			//����������socket�����뼯�ϣ�ֻ��Ҫ�����һ��socket
			FD_SET(_sock, &fdRead);

			timeval t = { 0,10 };
			int ret = select(_sock + 1, &fdRead, 0, 0, &t); //
			if (ret < 0)
			{
				printf("Accept Select���������\n");
				Close();
				return false;
			}

			//�ж���������socket���Ƿ��ڼ�����
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

	//�Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//���㲢���ÿ���յ���������Ϣ
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

	//ֻ�ᱻһ���̴߳��� ��ȫ
	virtual void OnNetJoin(DataClient* pClient)
	{
		_clientCount++;
		//printf("client<%d>  join\n", pClient->sockfd());
	}

	//cellServer 4 ����̴߳��� ����ȫ�����ֻ����1��cellServer���ǰ�ȫ��
	virtual void OnNetLeave(DataClient* pClient)
	{
		_clientCount--;
		//printf("client<%d>   leave\n", pClient->sockfd());
	}

	//cellServer 4 ����̴߳��� ����ȫ�����ֻ����1��cellServer���ǰ�ȫ��
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
