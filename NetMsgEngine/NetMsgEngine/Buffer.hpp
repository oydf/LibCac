/*
	��������
*/
#ifndef _BUFFER_H
#define _BUFFER_H
#include"Common.hpp"

class Buffer
{
private:
	//������
	char* _pBuff;
	//��������дָ��λ��
	int _nLast;
	//�������ܵĿռ��С���ֽڳ���
	int _nSize;
	//������д����������
	int _fullCount;
public:
	Buffer(int nSize = 8192):_nLast(0),_nSize(nSize),_fullCount(0)
	{
		_pBuff = new char[_nSize];
	}

	~Buffer()
	{
		if (_pBuff)
		{
			delete[] _pBuff;
			_pBuff = nullptr;
		}
	}

	char* data()
	{
		return _pBuff;
	}

	bool push(const char* pData, int nLen)
	{
		if (_nLast + nLen <= _nSize)
		{
			//��Ҫ���͵����� ���������ͻ�����β��
			memcpy(_pBuff + _nLast, pData, nLen);
			//��������β��λ��
			_nLast += nLen;

			if (_nLast == SEND_BUFF_SZIE)
			{
				++_fullCount;
			}

			return true;
		}
		else {
			++_fullCount;
		}

		return false;
	}

	void pop(int nLen)
	{
		int n = _nLast - nLen;
		if (n > 0)
		{
			memcpy(_pBuff, _pBuff + nLen, n);
		}
		_nLast = n;
		if (_fullCount > 0)
			--_fullCount;
	}

	int write2socket(SOCKET sockfd)
	{
		int ret = 0;
		//������������
		if (_nLast > 0 && INVALID_SOCKET != sockfd)
		{
			//��������
			ret = send(sockfd, _pBuff, _nLast, 0);
			//����β��λ������
			_nLast = 0;
			//
			_fullCount = 0;
		}
		return ret;
	}
	int read4socket(SOCKET sockfd)
	{
		if (_nSize - _nLast > 0)
		{
			//���տͻ�������
			char* szRecv = _pBuff + _nLast;
			int nLen = (int)recv(sockfd, szRecv, _nSize - _nLast, 0);
			//CELLLog::Info("nLen=%d\n", nLen);
			if (nLen <= 0)
			{
				return nLen;
			}
			//��Ϣ������������β��λ�ú���
			_nLast += nLen;
			return nLen;
		}
		return 0;
	}
	bool hasMsg()
	{
		//�ж���Ϣ�����������ݳ��ȴ�����Ϣͷnetmsg_DataHeader����
		if (_nLast >= sizeof(DataHeader))
		{
			//��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
			DataHeader* header = (DataHeader*)_pBuff;
			//�ж���Ϣ�����������ݳ��ȴ�����Ϣ����
			return _nLast >= header->dataLength;
		}
		return false;
	}
	bool needWrite()
	{
		return _nLast > 0;
	}
};

#endif // !_BUFFER_H
