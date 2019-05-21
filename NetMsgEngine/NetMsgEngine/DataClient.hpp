/*
	封装客户端连接，句柄、双缓冲区、读指针
*/
#ifndef _DATACLIENT_H
#define _DATACLIENT_H
#include"Common.hpp"

class DataClient
{
private:
	// 连接句柄
	SOCKET _sockfd;
	//第二缓冲区 消息缓冲区
	char _szMsgBuf[RECV_BUFF_SZIE];
	//消息缓冲区的数据尾部位置
	int _lastPos;
	//第二缓冲区 发送缓冲区
	char _szSendBuf[SEND_BUFF_SZIE];
	//发送缓冲区的数据尾部位置
	int _lastSendPos;
public:
	DataClient(SOCKET sockfd = INVALID_SOCKET)
	{
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, RECV_BUFF_SZIE);
		_lastPos = 0;
		memset(_szSendBuf, 0, SEND_BUFF_SZIE);
		_lastSendPos = 0;
	}
	SOCKET sockfd()
	{
		return _sockfd;
	}
	char* msgBuf()
	{
		return _szMsgBuf;
	}
	int getLastPos()
	{
		return _lastPos;
	}
	void setLastPos(int pos)
	{
		_lastPos = pos;
	}
	//发送数据
	int SendData(DataHeader* header)
	{
		int ret = SOCKET_ERROR;
		//要发送的数据长度
		int nSendLen = header->dataLength;
		//要发送的数据
		const char* pSendData = (const char*)header;
		//循环地装满第二缓冲区进行发送数据
		while (true)
		{
			if (_lastSendPos + nSendLen >= SEND_BUFF_SZIE)
			{
				//计算可拷贝的数据长度
				int nCopyLen = SEND_BUFF_SZIE - _lastSendPos;
				//拷贝数据
				memcpy(_szSendBuf + _lastSendPos, pSendData, nCopyLen);
				//计算剩余数据位置
				pSendData += nCopyLen;
				//计算剩余数据长度
				nSendLen -= nCopyLen;
				//发送数据
				ret = send(_sockfd, _szSendBuf, SEND_BUFF_SZIE, 0);
				//数据尾部位置清零
				_lastSendPos = 0;
				//发送错误
				if (SOCKET_ERROR == ret)
				{
					return ret;
				}
			}
			else 
			{
				//将要发送的数据 拷贝到发送缓冲区尾部
				memcpy(_szSendBuf + _lastSendPos, pSendData, nSendLen);
				//计算数据尾部位置
				_lastSendPos += nSendLen;
				break;
			}
		}
		return ret;
	}
};
#endif // _DATACLIENT_H