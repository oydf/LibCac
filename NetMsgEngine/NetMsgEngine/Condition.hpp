/*
	ʹ����������+�������߳�֮���˳����
	��֤�̰߳�ȫ���ŵ��˳�����־˳���Ե�
*/
#ifndef _CONDITION_H
#define _CONDITION_H
#include<chrono>
#include<condition_variable>

//�ź���
class Condition
{
private:
	//�ı����ݻ�����ʱ��Ҫ����
	std::mutex _mutex;
	//�����ȴ�-��������
	std::condition_variable _cv;
	//�ȴ�����
	int _wait = 0;
	//���Ѽ���
	int _wakeup = 0;
public:
	//������ǰ�߳�
	void wait()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		if (--_wait < 0)
		{
			//�����ȴ�
			_cv.wait(lock, [this]()->bool {
				return _wakeup > 0;
			});
			--_wakeup;
		}
	}

	//���ѵ�ǰ�߳�
	void wakeup()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (++_wait <= 0)
		{
			++_wakeup;
			_cv.notify_one();
		}
	}
};

#endif // !_CONDITION_H

//��ٻ���