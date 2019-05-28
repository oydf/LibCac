/*
	生产者消费者模式多线程处理自定义任务（可加线程池）
*/
#ifndef _VIRTUALTASK_H_
#define _VIRTUALTASK_H_
#include"Thread.hpp"
#include<mutex>
#include<vector>
//任务基类
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
	//任务队列
	std::vector<VirtualTask* > _tasks;
	//任务缓冲区
	std::vector<VirtualTask* > _tasksBuff;
	//加锁
	std::mutex _mutex;
	mThread _thread;
public:
	int serverid = -1;
	//添加任务
	void addTask(VirtualTask* task)
	{
		std::lock_guard<std::mutex> tkl(_mutex);
		_tasksBuff.push_back(task);
	}

	//启动工作线程
	void Start()
	{
		_thread.Start(nullptr, [this](mThread* pThread) {
			Run(pThread);
		});
	}
	void close()
	{
		_thread.Close();
	}
protected:
	//从缓冲区取出任务，循环处理
	void Run(mThread* pThread)
	{
		while (pThread->isRun())
		{
			//从缓冲区取出数据
			if (!_tasksBuff.empty())
			{
				std::lock_guard<std::mutex> tkl(_mutex);
				for (auto pTask : _tasksBuff)
				{
					_tasks.push_back(pTask);
				}
				_tasksBuff.clear();
			}
			//如果没有任务
			if (_tasks.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			//处理任务
			for (auto pTask : _tasks)
			{
				pTask->myTask();
				delete pTask;
			}
			//清空任务
			_tasks.clear();
		}
		for (auto pTask : _tasksBuff)
		{
			pTask->myTask();
		}
	}
};

#endif //_VIRTUALTASK_H_