/*
	在多端口同时使用的情况下，win环境初始化只一次，采用单例模式
*/

#ifndef _WORKENV_H
#define _WORKENV_H

#include"Common.hpp"

class WorkEnv
{
private:
	WorkEnv()
	{
#ifdef _WIN32
		//启动Windows socket 2.x环境
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
#ifndef _WIN32
		signal(SIGPIPE, SIG_IGN);
#endif
	}

	~WorkEnv()
	{
#ifdef _WIN32
		//清除环境
		WSACleanup();
#endif
	}

public:
	static void Init()
	{
		static  WorkEnv obj;
	}
};

#endif // !_WORKENV_H
