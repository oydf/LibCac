#include "MainServer.hpp"
#include<thread>
#include<cstdlib>
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
			printf("�˳�cmdThread�߳�\n");
			break;
		}
		else 
		{
			printf("��֧�ֵ����\n");
		}
	}
}

class MyServer : public MainServer
{
public:
	//ֻ�ᱻһ���̴߳��� ��ȫ
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
	
	virtual void OnNetMsg(ResponseServer* pCellServer, DataClient* pClient, DataHeader* header)
	{
		MainServer::OnNetMsg(pCellServer, pClient, header);
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			pClient->resetDTHeart();
			Login* login = (Login*)header;
			//printf("�յ��ͻ���<Socket=%d>����CMD_LOGIN,���ݳ��ȣ�%d,userName=%s PassWord=%s\n", cSock, login->dataLength, login->userName, login->PassWord);
			//�����ж��û������Ƿ���ȷ�Ĺ���
			LoginR ret;
			pClient->SendData(&ret);
			//LoginR* ret = new LoginR();
			//pCellServer->addSendTask(pClient, ret);
		}//���� ��Ϣ---���� ����   ������ ���ݻ�����  ������ 
		break;
		case CMD_CTOS_HERAT:
		{
			pClient->resetDTHeart();
			cTosHeart ret;
			pClient->SendData(&ret);
		}
		break;
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			//printf("�յ��ͻ���<Socket=%d>����CMD_LOGOUT,���ݳ��ȣ�%d,userName=%s \n", cSock, logout->dataLength, logout->userName);
			//�����ж��û������Ƿ���ȷ�Ĺ���
			//netmsg_LogoutR ret;
			//SendData(cSock, &ret);
		}
		break;
		default:
		{
			printf("<socket=%d>�յ�δ������Ϣ,���ݳ��ȣ�%d\n", pClient->sockfd(), header->dataLength);
			//netmsg_DataHeader ret;
			//SendData(cSock, &ret);
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
	server.Listen(5);
	server.Start(1);

	//����UI�߳�
	std::thread t1(cmdThread);
	t1.detach();

	while (g_bRun)
	{
		server.Run();
		//printf("����ʱ�䴦������ҵ��..\n");
	}
	server.Close();
	printf("���˳���\n");
	getchar();
	return 0;
}