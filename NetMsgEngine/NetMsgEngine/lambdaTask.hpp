#ifndef _LAMBDATASK_H
#define _LAMBDATASK_H

#include<mutex>
#include<list>
#include<functional>
#include"Thread.hpp"
using namespace std;
//ִ������ķ�������
class LambdaTask
{
public:
	//����serverid
	int serverId = -1;
private:
	typedef std::function<void()> CELLTask;
private:
	//��������
	std::list<CELLTask> _tasks;
	//�������ݻ�����
	std::list<CELLTask> _tasksBuf;
	//�ı����ݻ�����ʱ��Ҫ����
	std::mutex _mutex;

	
public:
	mThread _thread;
	//�������
	void addTask(CELLTask task)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_tasksBuf.push_back(task);
	}
	//���������߳�
	void Start()
	{
		_thread.Start(nullptr, [this](mThread* pThread) {
			OnRun(pThread);
		});
	}

	void Close()
	{
		///CELLLog::Info("LambdaTask%d.Close begin\n", serverId);
		_thread.Close();
		//CELLLog::Info("LambdaTask%d.Close end\n", serverId);
	}
protected:
	//��������
	void OnRun(mThread* pThread)
	{
		while (pThread->isRun())
		{
			//�ӻ�����ȡ������
			if (!_tasksBuf.empty())
			{
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pTask : _tasksBuf)
				{
					_tasks.push_back(pTask);
				}
				_tasksBuf.clear();
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
				pTask();
			}
			//�������
			_tasks.clear();
		}
		//����������е�����
		for (auto pTask : _tasksBuf)
		{
			pTask();
		}
		//CELLLog::Info("LambdaTask%d.OnRun exit\n", serverId);
	}
};
#endif // _LAMBDATASK_H
