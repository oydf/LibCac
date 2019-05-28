#include"Logger.hpp"
#include<windows.h>
#include<iostream>
using namespace std;
int main()
{
	Logger::Instance().setLogPath("serverLog.txt", "w");
	
	while (true)
	{
		Sleep(2000);
		Logger::Info("undefine cmd\n");
	}
	system("pause");
	return 0;
}
