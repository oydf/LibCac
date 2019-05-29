#include"MainClient.hpp"
#include"Timer.hpp"
#include<atomic>


class MyClient : public MainClient
{
public:
	//响应网络消息
	virtual void OnNetMsg(DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginR* login = (LoginR*)header;
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			LogoutR* logout = (LogoutR*)header;
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin* userJoin = (NewUserJoin*)header;
			//LOG_INFO("<socket=%d> recv msgType：CMD_NEW_USER_JOIN\n", (int)_pClient->sockfd());
		}
		break;
		case CMD_ERROR:
		{
			LOG_INFO("<socket=%d> recv msgType：CMD_ERROR\n", (int)_pClient->sockfd());
		}
		break;
		default:
		{
			LOG_INFO("error, <socket=%d> recv undefine msgType\n", (int)_pClient->sockfd());
		}
		}
	}
private:

};


bool g_bRun = true;
void cmdThread()
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			LOG_INFO("退出cmdThread线程\n");
			break;
		}
		else {
			LOG_INFO("不支持的命令。\n");
		}
	}
}

//客户端数量
const int cCount = 1000;
//发送线程数量
const int tCount = 4;
//客户端数组
MainClient* client[cCount];
std::atomic_int sendCount(0);
std::atomic_int readyCount(0);

void recvThread(int begin, int end)
{
	//CELLTimestamp t;
	while (g_bRun)
	{
		for (int n = begin; n < end; n++)
		{
			//if (t.getElapsedSecond() > 3.0 && n == begin)
			//	continue;
			client[n]->OnRun();
		}
	}
}

void sendThread(int id)
{
	LOG_INFO("thread<%d>,start\n", id);
	int c = cCount / tCount;
	int begin = (id - 1)*c;
	int end = id*c;

	for (int n = begin; n < end; n++)
	{
		client[n] = new MyClient();
	}
	for (int n = begin; n < end; n++)
	{
		client[n]->Connect("192.168.1.102", 4567);
	}
	//心跳检测 死亡计时 
	LOG_INFO("thread<%d>,Connect<begin=%d, end=%d>\n", id, begin, end);

	readyCount++;
	while (readyCount < tCount)
	{
		//等待其它线程准备好发送数据
		std::chrono::milliseconds t(10);
		std::this_thread::sleep_for(t);
	}
	std::thread t1(recvThread, begin, end);
	t1.detach();
	Login login[1];
	for (int n = 0; n < 1; n++)
	{
		strcpy(login[n].userName, "lyd");
		strcpy(login[n].PassWord, "lydmm");
	}
	const int nLen = sizeof(login);

	while (g_bRun)
	{
		for (int n = begin; n < end; n++)
		{
			if (SOCKET_ERROR != client[n]->SendData(login))
			{
				sendCount++;
			}
		}
		std::chrono::milliseconds t(99);
		std::this_thread::sleep_for(t);
	}

	for (int n = begin; n < end; n++)
	{
		client[n]->Close();
		delete client[n];
	}

	LOG_INFO("thread<%d>,exit\n", id);
}

int main()
{
	Logger::Instance().setLogPath("clientLog.txt", "w");
	Logger::setLogFile(false);
	std::thread t1(cmdThread);
	t1.detach();

	//启动发送线程
	for (int n = 0; n < tCount; n++)
	{
		std::thread t1(sendThread,n+1);
		t1.detach();
	}

	Timer tTime;

	while (g_bRun)
	{
		auto t = tTime.getSecondsInterval();
		if (t >= 1.0)
		{
			LOG_INFO("thread<%d>,clients<%d>,time<%lf>,send<%d>\n",tCount, cCount,t,(int)(sendCount/ t));
			sendCount = 0;
			tTime.reset();
		}
		std::chrono::milliseconds ts(1);
		std::this_thread::sleep_for(ts);
	}

	LOG_INFO("已退出。\n");
	return 0;
}
