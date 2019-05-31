/*
	c struct���͵���Ϣ�ṹ����չ�ԺͿ�ά����̫�ͣ��Ҳ���ƽ̨
*/
//ʹ�ÿ��ֽ����ͣ���ϵͳ������
#include<cstdint>
#include"Logger.hpp"
class ByteStream
{
private:
	//������
	char* _pBuff ;
	//�ֽڳ���
	int _nSize = 0;
	//��д�����ݵ�β��λ��
	int _nWritePos = 0;
	//�Ѷ�ȡ���ݵ�β��λ��
	int _nReadPos = 0;
	//_pBuff���ⲿ��������ݿ�ʱ�Ƿ�Ӧ�ñ��ͷ�
	bool _bDelete ;
public:
	ByteStream(char* pData, int nSize, bool bDelete = false):
		_pBuff(pData),_nSize(nSize),_nWritePos(0),_nReadPos(0),_bDelete(bDelete){}

	ByteStream(int nSize = 1024):
		_nSize(nSize), _nWritePos(0), _nReadPos(0),_bDelete(true)
	{
		_pBuff = new char[_nSize];
	}

	virtual ~ByteStream()
	{
		if (_bDelete && _pBuff)
		{
			delete[] _pBuff;
			_pBuff = nullptr;
		}
	}

	char* getData()
	{
		return _pBuff;
	}

	int getLength()
	{
		return _nWritePos;
	}

	bool canRead(int n)
	{
		return _nSize - _nReadPos >= n;
	}

	bool canWrite(int n)
	{
		return _nSize - _nWritePos >= n;
	}

	//д��λ������
	void moveWrite(int n)
	{
		_nWritePos += n;
	}
	//����λ������
	void moveRead(int n)
	{
		_nReadPos += n;
	}
	//����д��λ��
	void setWritePos(int n)
	{
		_nWritePos = n;
	}

	//��ģ�巽�������ظ�����
	//���ֽڶ�ȡ
	template<typename T>
	bool Read(T& data , bool Offset = true)
	{
		auto nLen = sizeof(T);
		if (canRead(nLen))
		{
			memcpy(&data, _pBuff + _nReadPos, nLen);
			if (Offset)
				moveRead(nLen);
			return true;
		}
		LOG_INFO("error,ByteStream::Read failed.");
		return false;
	}

	//�����ȡ
	template<typename T>
	uint32_t ReadArray(T* pArr, uint32_t len)
	{
		uint32_t length = 0;
		//д������ʱ����һ��Ԫ�ش洢���鳤��
		Read(length, false);
		//�жϻ��������ܷ�ŵ���
		if (length < len)
		{
			//����������ֽڳ���
			auto nLen = length * sizeof(T);
			//�ж��ܲ��ܶ���
			if (canRead(nLen + sizeof(uint32_t)))
			{
				//�����Ѷ�λ��+���鳤����ռ�пռ�
				moveRead(sizeof(uint32_t));
				//��Ҫ��ȡ������ ��������
				memcpy(pArr, _pBuff + _nReadPos, nLen);
				//�����Ѷ�����λ��
				moveRead(nLen);
				return length;
			}
		}
		LOG_INFO("error, ByteStream::ReadArray failed.");
		return 0;
	}

	template<typename T>
	bool Write(T n)
	{
		//����Ҫд�����ݵ��ֽڳ���
		auto nLen = sizeof(T);
		//�ж��ܲ���д��
		if (canWrite(nLen))
		{
			//��Ҫд������� ������������β��
			memcpy(_pBuff + _nWritePos, &n, nLen);
			//������д������β��λ��
			moveWrite(nLen);
			return true;
		}
		LOG_INFO("error, ByteStream::Write failed.");
		return false;
	}

	template<typename T>
	bool WriteArray(T* pData, uint32_t len)
	{
		//����Ҫд��������ֽڳ���
		auto nLen = sizeof(T)*len;
		//�ж��ܲ���д��
		if (canWrite(nLen + sizeof(uint32_t)))
		{
			//��д�������Ԫ������
			Write(len);
			//��Ҫд������� ������������β��
			memcpy(_pBuff + _nWritePos, pData, nLen);
			//��������β��λ��
			push(nLen);
			return true;
		}
		LOG_INFO("error, ByteStream::WriteArray failed.");
		return false;
	}
};