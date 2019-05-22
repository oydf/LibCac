/*
	����ͨ�ŵ��������¼��������û��Զ���������
*/
#ifndef _NETEVENT_H
#define _NETEVENT_H
#include"Common.hpp"
#include"DataClient.hpp"

class ResponseServer;
class NetEvent
{
public:
	//�ͻ��˼����¼�
	virtual void OnNetJoin(DataClient* pClient) = 0;
	//�ͻ����뿪�¼�
	virtual void OnNetLeave(DataClient* pClient) = 0;
	//�ͻ�����Ϣ�¼�
	virtual void OnNetMsg(ResponseServer* pCellServer, DataClient* pClient, DataHeader* header) = 0;
	//recv�¼�
	virtual void OnNetRecv(DataClient* pClient) = 0;
};

#endif // !_NETEVENT_H
