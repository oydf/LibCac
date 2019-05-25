/*
	������������ģʽ���̴߳����Զ������񣨿ɼ��̳߳أ�
*/
#ifndef _VIRTUALTASK_H_
#define _VIRTUALTASK_H_
#include"Thread.hpp"
#include<mutex>
#include<vector>
//�������
class VirtualTask
{
private:
public:
	VirtualTask() {}
	virtual ~VirtualTask() {}
	virtual void myTask() {}
};

class TaskServer
{
private:
	//�������
	std::vector<VirtualTask* > _tasks;
	//���񻺳���
	std::vector<VirtualTask* > _tasksBuff;
	//����
	std::mutex _mutex;

	mThread _thread;
public:
	//�������
	void addTask(VirtualTask* task)
	{
		std::lock_guard<std::mutex> tkl(_mutex);
		_tasksBuff.push_back(task);
	}
	//���������߳�
	void Start()
	{
		//�߳�
		std::thread t(std::mem_fn(&TaskServer::Run), this);
		t.detach();
	}
	void close()
	{
		_thread.Close();
	}
protected:
	//�ӻ�����ȡ������ѭ������
	void Run()
	{
		while (true)
		{
			//�ӻ�����ȡ������
			if (!_tasksBuff.empty())
			{
				std::lock_guard<std::mutex> tkl(_mutex);
				for (auto pTask : _tasksBuff)
				{
					_tasks.push_back(pTask);
				}
				_tasksBuff.clear();
			}
			//���û������
			if (_tasks.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			//��������
			for (auto pTask : _tasks)
			{
				pTask->myTask();
				delete pTask;
			}
			//�������
			_tasks.clear();
		}
	}
};

#endif //_VIRTUALTASK_H_