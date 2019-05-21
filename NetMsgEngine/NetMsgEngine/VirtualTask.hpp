/*
	生产者消费者模式多线程处理自定义任务（可加线程池）
*/
#ifndef _VIRTUALTASK_H_
#define _VIRTUALTASK_H_
#include<thread>
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
public:
	//添加任务
	void addTask(VirtualTask* task)
	{
		std::lock_guard<std::mutex> tkl(_mutex);
		_tasksBuff.push_back(task);
	}
	//启动工作线程
	void Start()
	{
		//线程
		std::thread t(std::mem_fn(&TaskServer::Run), this);
		t.detach();
	}
protected:
	//从缓冲区取出任务，循环处理
	void Run()
	{
		while (true)
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
	}
};

#endif //_VIRTUALTASK_H_