/*
	�ڶ�˿�ͬʱʹ�õ�����£�win������ʼ��ֻһ�Σ����õ���ģʽ
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
		//����Windows socket 2.x����
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
		//�������
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
