/*
	����ʹ��lambda���ʽ��ע�������񣬼���new����Ͷ�̬�����Ŀ���
*/

#ifndef _SENDTASK_H
#define _SENDTASK_H
#include"Common.hpp"
#include"NetEvent.hpp"
#include"DataClient.hpp"
#include<vector>
#include<map>

//������Ϣ��������
class SendTask:public VirtualTask
{
private:
	DataClient* _pClient;
	DataHeader* _pHeader;
public:
	SendTask(DataClient* pClient, DataHeader* header)
	{
		_pClient = pClient;
		_pHeader = header;
	}

	//ִ������
	void myTask()
	{
		_pClient->SendData(_pHeader);
		delete _pHeader;
	}
};

#endif //_SENDTASK_H