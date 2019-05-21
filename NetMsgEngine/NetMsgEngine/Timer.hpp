/*
	chrono是一个c++11的时间模板库，精确到纳秒级别。
	std::chrono::duration 表示一段时间；
	std::chrono::time_point 表示一个具体时间；
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

	//获取毫秒级时间
	static time_t getCurMill()
	{
		return duration_cast<milliseconds>(h_t_p::now().time_since_epoch()).count();
	}

	Timer()
	{
		_cur = h_t_p::now();
	}

	~Timer() {}

	//获取微秒间隔
	long long getMicrosecondsInterval()
	{
		return duration_cast<microseconds>(h_t_p::now() - _cur).count();
	}

	//获取毫秒间隔
	double getMillsecondsInterval()
	{
		return this->getMicrosecondsInterval() * 0.001;
	}

	//获取秒间隔
	double getSecondsInterval()
	{
		return  getMicrosecondsInterval() * 0.000001;
	}

	//获取当前秒
	double getCurSecond()
	{
		return  duration_cast<microseconds>(h_t_p::now().time_since_epoch()).count() * 0.000001;
	}

	//重置计时器
	void reset()
	{
		_cur = h_t_p::now();
	}
};

#endif // !_TIMER_H
