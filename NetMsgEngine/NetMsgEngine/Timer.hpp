/*
	chrono��һ��c++11��ʱ��ģ��⣬��ȷ�����뼶��
	std::chrono::duration ��ʾһ��ʱ�䣻
	std::chrono::time_point ��ʾһ������ʱ�䣻
*/

#ifndef _TIMER_H
#define _TIMER_H

#include<chrono>
using namespace std::chrono;

typedef std::chrono::high_resolution_clock   h_t_p;

class Timer 
{
protected:
	time_point<high_resolution_clock> _cur;
public:

	//��ȡ���뼶ʱ��
	static time_t getCurMill()
	{
		return duration_cast<milliseconds>(h_t_p::now().time_since_epoch()).count();
	}

	Timer()
	{
		_cur = h_t_p::now();
	}

	~Timer() {}

	//��ȡ΢����
	long long getMicrosecondsInterval()
	{
		return duration_cast<microseconds>(h_t_p::now() - _cur).count();
	}

	//��ȡ������
	double getMillsecondsInterval()
	{
		return this->getMicrosecondsInterval() * 0.001;
	}

	//��ȡ����
	double getSecondsInterval()
	{
		return  getMicrosecondsInterval() * 0.000001;
	}

	//��ȡ��ǰ��
	double getCurSecond()
	{
		return  duration_cast<microseconds>(h_t_p::now().time_since_epoch()).count() * 0.000001;
	}

	//���ü�ʱ��
	void reset()
	{
		_cur = h_t_p::now();
	}
};

#endif // !_TIMER_H
