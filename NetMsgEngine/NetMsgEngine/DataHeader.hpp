/*
	��������ͷ��������������ݳ��ȣ������������ͷ��
	���������շ����ܵļ������ݰ�
*/
#ifndef _DATAHEADER_H
#define _DATAHEADER_H

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_CTOS_HERAT,
	CMD_STOC_HERAT,
	CMD_ERROR
};
//����ͷ����
struct DataHeader
{
	DataHeader()
	{
		dataLength = sizeof(DataHeader);
		cmd = CMD_ERROR;
	}
	short dataLength;
	CMD cmd;
};
//��¼
struct Login : public DataHeader
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];
	char data[32];
};

//��¼���
struct LoginR : public DataHeader
{
	LoginR()
	{
		dataLength = sizeof(LoginR);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
	char data[92];
};

//ע��
struct Logout : public DataHeader
{
	Logout()
	{
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};

//�ظ��û�������Ϣ
struct NewUserJoin : public DataHeader
{
	NewUserJoin()
	{
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		scok = 0;
	}
	int scok;
};

//ע�����
struct LogoutR : public DataHeader
{
	LogoutR()
	{
		dataLength = sizeof(LogoutR);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}
	int result;
};

struct cTosHeart : public DataHeader
{
	cTosHeart()
	{
		dataLength = sizeof(cTosHeart);
		cmd = CMD_CTOS_HERAT;
	}
};

struct sTocHeart : public DataHeader
{
	sTocHeart()
	{
		dataLength = sizeof(sTocHeart);
		cmd = CMD_STOC_HERAT;
	}
};


#endif //_DATAHEADER_H