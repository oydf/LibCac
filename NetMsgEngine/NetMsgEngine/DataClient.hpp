/*
	��װ�ͻ������ӣ������˫����������ָ��
*/
#ifndef _DATACLIENT_H
#define _DATACLIENT_H
#include"Common.hpp"
#include"Buffer.hpp"

//�ͻ����������������ʱʱ��
#define CLIENT_HREAT_DEAD_TIME 5000
#define CLIENT_SEND_BUFF_TIME 200

class DataClient
{
private:
	// ���Ӿ��
	SOCKET _sockfd;
	//	������Ϣ������
	Buffer _recvBuff;
	// ���ͻ�����
	Buffer _sendBuff;
	//����������ʱ
	time_t _dtHeart;
	//�ϴη�����Ϣ���ݵ�ʱ�� 
	time_t _dtSend;
	//���ͻ���������д���������
	int _sendBuffFullCount = 0;
public:
	int cid;
	int sid;

	DataClient(SOCKET sockfd = INVALID_SOCKET) :
		_sendBuff(SEND_BUFF_SZIE),
		_recvBuff(RECV_BUFF_SZIE),
		_sockfd(sockfd)
	{
		static int n = 1;
		cid = n++;
		resetDTHeart();
		resetDTSend();
	}

	~DataClient()
	{
		if (INVALID_SOCKET != _sockfd)
		{
#ifdef _WIN32
			closesocket(_sockfd);
#else
			close(_sockfd);
#endif
			_sockfd = INVALID_SOCKET;
		}
	}


	SOCKET sockfd()
	{
		return _sockfd;
	}

	int RecvData()
	{
		return _recvBuff.read4socket(_sockfd);
	}

	bool hasMsg()
	{
		return _recvBuff.hasMsg();
	}

	DataHeader* front_msg()
	{
		return (DataHeader*)_recvBuff.data();
	}

	void pop_front_msg()
	{
		if (hasMsg())
			_recvBuff.pop(front_msg()->dataLength);
	}

	bool needWrite()
	{
		return _sendBuff.needWrite();
	}

	int SendDataReal()
	{
		resetDTSend();
		return _sendBuff.write2socket(_sockfd);
	}

	int SendData(DataHeader* header)
	{
		if (_sendBuff.push((const char*)header, header->dataLength))
		{
			return header->dataLength;
		}
		return SOCKET_ERROR;
	}

	void resetDTHeart()
	{
		_dtHeart = 0;
	}

	void resetDTSend()
	{
		_dtSend = 0;
	}

	bool checkHeart(time_t dt)
	{
		_dtHeart += dt;
		if (_dtHeart >= CLIENT_HREAT_DEAD_TIME)
		{
			return true;
		}
		return false;
	}

	//��ʱ������Ϣ���
	bool checkSend(time_t dt)
	{
		_dtSend += dt;
		if (_dtSend >= CLIENT_SEND_BUFF_TIME)
		{
			//CELLLog::Info("checkSend:s=%d,time=%d\n", _sockfd, _dtSend);
			//���������ͻ����������ݷ��ͳ�ȥ
			SendDataReal();
			//���÷��ͼ�ʱ
			resetDTSend();
			return true;
		}
		return false;
	}
};
#endif // _DATACLIENT_H