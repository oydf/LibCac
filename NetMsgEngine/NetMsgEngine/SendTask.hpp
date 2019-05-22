/*
	考虑使用lambda表达式来注册新任务，减少new对象和多态带来的开销
*/

#ifndef _SENDTASK_H
#define _SENDTASK_H
#include"Common.hpp"
#include"NetEvent.hpp"
#include"DataClient.hpp"
#include<vector>
#include<map>

//网络消息发送任务
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

	//执行任务
	void myTask()
	{
		_pClient->SendData(_pHeader);
		delete _pHeader;
	}
};

#endif //_SENDTASK_H