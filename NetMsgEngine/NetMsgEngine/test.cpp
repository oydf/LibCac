#include "MainServer.hpp"
#include<thread>

class MyServer : public MainServer
{
public:

	//cellServer 4 ����̴߳��� ����ȫ
	//���ֻ����1��cellServer���ǰ�ȫ��
	virtual void OnNetJoin(DataClient* pClient)
	{
		MainServer::OnNetJoin(pClient);
	}
	//cellServer 4 ����̴߳��� ����ȫ
	//���ֻ����1��cellServer���ǰ�ȫ��
	virtual void OnNetLeave(DataClient* pClient)
	{
		MainServer::OnNetLeave(pClient);
	}
	//cellServer 4 ����̴߳��� ����ȫ
	//���ֻ����1��cellServer���ǰ�ȫ��
	virtual void OnNetMsg(ResponseServer* pServer, DataClient* pClient,DataHeader* header)
	{
		MainServer::OnNetMsg(pServer, pClient, header);
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			pClient->resetDTHeart();
			//send recv 
			Login* login = (Login*)header;
			//CELLLog::Info("recv <Socket=%d> msgType��CMD_LOGIN, dataLen��%d,userName=%s PassWord=%s\n", cSock, login->dataLength, login->userName, login->PassWord);
			//�����ж��û������Ƿ���ȷ�Ĺ���
		    LoginR ret;
			if (SOCKET_ERROR == pClient->SendData(&ret))
			{
				//���ͻ��������ˣ���Ϣû����ȥ
			}
			//netmsg_LoginR* ret = new netmsg_LoginR();
			//pServer->addSendTask(pClient, ret);
		}//���� ��Ϣ---���� ����   ������ ���ݻ�����  ������ 
		break;
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			//CELLLog::Info("recv <Socket=%d> msgType��CMD_LOGOUT, dataLen��%d,userName=%s \n", cSock, logout->dataLength, logout->userName);
			//�����ж��û������Ƿ���ȷ�Ĺ���
			//netmsg_LogoutR ret;
			//SendData(cSock, &ret);
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
			
		}
		break;
		}
	}

private:

};

int main()
{
	MyServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(64);
	server.Start(4);

	//�����߳��еȴ��û���������
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			server.Close();
			break;
		}
		else {
			//CELLLog::Info("undefine cmd\n");
		}
	}

	//CELLLog::Info("exit.\n");
	//#ifdef _WIN32
	//	while (true)
	//		Sleep(10);
	//#endif
	return 0;
}
