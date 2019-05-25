/*
	封装Thead,更优雅的管控线程，线程函数的注册更灵活
*/
#ifndef _THREAD_H
#define _THREAD_H
#include"Condition.hpp"
#include<thread>
#include<functional>

//向启动线程注册回调函数
class mThread;
typedef std::function<void(mThread*)> _ThreadCall;

class mThread
{
private:
	//单次执行函数
	_ThreadCall _create;
	//eventloop函数
	_ThreadCall _loop;
	//销毁函数
	_ThreadCall _destory;
	//不同线程中改变数据时需要加锁
	std::mutex _mutex;
	//控制线程的终止、退出
	Condition _sem;
	//线程是否启动运行中
	bool _isRun;	
public:
	mThread() :_isRun(false) {}

	//启动线程
	void Start(_ThreadCall onCreate = nullptr, _ThreadCall onRun = nullptr,_ThreadCall onDestory = nullptr)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (!_isRun)
		{
			_isRun = true;
			if (_create)
				_create = onCreate;
			if (_loop)
				_loop = onRun;
			if (_destory)
				_destory = onDestory;
			//线程
			std::thread t(std::mem_fn(&mThread::OnWork), this);
			t.detach();
		}
	}

	//关闭线程
	void Close()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			_isRun = false;
			_sem.wait();
		}
	}
	//在工作函数中退出
	//不需要使用信号量来阻塞等待
	//如果使用会阻塞
	void Exit()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			_isRun = false;
		}
	}

	//线程是否启动运行状态
	bool isRun()
	{
		return _isRun;
	}
protected:
	//线程的运行时的工作函数
	void OnWork()
	{
		if (_create)
			_create(this);
		if (_loop)
			_loop(this);
		if (_destory)
			_destory(this);
		_sem.wakeup();
	}
};
#endif // !_THREAD_H
