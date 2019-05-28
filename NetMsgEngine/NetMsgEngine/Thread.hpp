/*
	��װThead,�����ŵĹܿ��̣߳��̺߳�����ע������
*/
#ifndef _THREAD_H
#define _THREAD_H
#include"Condition.hpp"
#include<thread>

//�������߳�ע��ص�����

class mThread
{
private:
	typedef std::function<void(mThread*)> _ThreadCall;
private:
	//����ִ�к���
	_ThreadCall _create;
	//eventloop����
	_ThreadCall _loop;
	//���ٺ���
	_ThreadCall _destory;
	//��ͬ�߳��иı�����ʱ��Ҫ����
	std::mutex _mutex;
	//�����̵߳���ֹ���˳�
	Condition _sem;
	//�߳��Ƿ�����������
	bool _isRun = false;	
public:
	//mThread() :_isRun(false) {}

	//�����߳�
	void Start(_ThreadCall onCreate = nullptr, 
		_ThreadCall onRun = nullptr,
		_ThreadCall onDestory = nullptr)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (!_isRun)
		{
			_isRun = true;
			if (onCreate)
				_create = onCreate;
			if (onRun)
				_loop = onRun;
			if (onDestory)
				_destory = onDestory;
			//�߳�
			std::thread t(std::mem_fn(&mThread::OnWork), this);
			t.detach();
		}
	}

	//�ر��߳�
	void Close()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			_isRun = false;
			_sem.wait();
		}
	}
	//�ڹ����������˳�
	//����Ҫʹ���ź����������ȴ�
	//���ʹ�û�����
	void Exit()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			_isRun = false;
		}
	}

	//�߳��Ƿ���������״̬
	bool isRun()
	{
		return _isRun;
	}
protected:
	//�̵߳�����ʱ�Ĺ�������
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
