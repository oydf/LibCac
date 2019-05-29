#include "MainServer.hpp"
#include"Logger.hpp"
class MyServer : public MainServer
{
public:
	virtual void OnNetJoin(DataClient* pClient)
	{
		MainServer::OnNetJoin(pClient);
	}
	virtual void OnNetLeave(DataClient* pClient)
	{
		MainServer::OnNetLeave(pClient);
	}
	virtual void OnNetMsg(ResponseServer* pServer, DataClient* pClient, DataHeader* header)
	{
		MainServer::OnNetMsg(pServer, pClient, header);
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			pClient->resetDTHeart();
			Login* login = (Login*)header;
			LoginR ret;
			if (SOCKET_ERROR == pClient->SendData(&ret))
			{
				LOG_INFO("<Socket=%d> Send Full\n", pClient->sockfd());
			}
			break;
		}
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
		}
		break;
		case CMD_CTOS_HERAT:
		{
			pClient->resetDTHeart();
			sTocHeart ret;
			pClient->SendData(&ret);
		}
		default:
		{
			LOG_INFO("recv <socket=%d> undefine msgType,dataLen：%d\n", pClient->sockfd(), header->dataLength);
		}
		break;
		}
	}
};

int main()
{
	Logger::Instance().setLogPath("serverLog.txt","w");
	MyServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(64);
	server.Start(4);
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			server.Close();
			break;
		}
		else 
		{
			LOG_INFO("undefine cmd\n");
		}
	}
	LOG_INFO("exit.\n");
	return 0;
}
