/*
	网络通信的三个半事件，交于用户自定义具体操作
*/
#ifndef _NETEVENT_H
#define _NETEVENT_H
#include"Common.hpp"
#include"DataClient.hpp"

class ResponseServer;
class NetEvent
{
public:
	//客户端加入事件
	virtual void OnNetJoin(DataClient* pClient) = 0;
	//客户端离开事件
	virtual void OnNetLeave(DataClient* pClient) = 0;
	//客户端消息事件
	virtual void OnNetMsg(ResponseServer* pCellServer, DataClient* pClient, DataHeader* header) = 0;
	//recv事件
	virtual void OnNetRecv(DataClient* pClient) = 0;
};

#endif // !_NETEVENT_H
