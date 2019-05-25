/*
	��װThead,�����ŵĹܿ��̣߳��̺߳�����ע������
*/
#ifndef _THREAD_H
#define _THREAD_H
#include"Condition.hpp"
#include<thread>

class Thread
{
private:
	typedef std::function<void(Thread*)> EventCall;
	EventCall _onCreate;
	EventCall _onRun;
	EventCall _onDestory;
	//��ͬ�߳��иı�����ʱ��Ҫ����
	std::mutex _mutex;
	//�����̵߳���ֹ���˳�
	CELLSemaphore _sem;
	//�߳��Ƿ�����������
	bool	_isRun = false;	
public:
	//�����߳�
	void Start(
		EventCall onCreate = nullptr,
		EventCall onRun = nullptr,
		EventCall onDestory = nullptr)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (!_isRun)
		{
			_isRun = true;

			if (onCreate)
				_onCreate = onCreate;
			if (onRun)
				_onRun = onRun;
			if (onDestory)
				_onDestory = onDestory;

			//�߳�
			std::thread t(std::mem_fn(&Thread::OnWork), this);
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
		if (_onCreate)
			_onCreate(this);
		if (_onRun)
			_onRun(this);
		if (_onDestory)
			_onDestory(this);

		_sem.wakeup();
	}

};


#endif // !_THREAD_H
