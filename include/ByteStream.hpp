/*
	c struct类型的消息结构可扩展性和可维护性太低，且不跨平台
*/
//使用宽字节类型，跨系统跨语言
#include<cstdint>
#include"Logger.hpp"
class ByteStream
{
private:
	//缓冲区
	char* _pBuff ;
	//字节长度
	int _nSize = 0;
	//已写入数据的尾部位置
	int _nWritePos = 0;
	//已读取数据的尾部位置
	int _nReadPos = 0;
	//_pBuff是外部传入的数据块时是否应该被释放
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

	//写入位置右移
	void moveWrite(int n)
	{
		_nWritePos += n;
	}
	//读出位置右移
	void moveRead(int n)
	{
		_nReadPos += n;
	}
	//设置写入位置
	void setWritePos(int n)
	{
		_nWritePos = n;
	}

	//用模板方法减少重复代码
	//单字节读取
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

	//数组读取
	template<typename T>
	uint32_t ReadArray(T* pArr, uint32_t len)
	{
		uint32_t length = 0;
		//写入数组时，第一个元素存储数组长度
		Read(length, false);
		//判断缓存数组能否放得下
		if (length < len)
		{
			//计算数组的字节长度
			auto nLen = length * sizeof(T);
			//判断能不能读出
			if (canRead(nLen + sizeof(uint32_t)))
			{
				//计算已读位置+数组长度所占有空间
				moveRead(sizeof(uint32_t));
				//将要读取的数据 拷贝出来
				memcpy(pArr, _pBuff + _nReadPos, nLen);
				//计算已读数据位置
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
		//计算要写入数据的字节长度
		auto nLen = sizeof(T);
		//判断能不能写入
		if (canWrite(nLen))
		{
			//将要写入的数据 拷贝到缓冲区尾部
			memcpy(_pBuff + _nWritePos, &n, nLen);
			//计算已写入数据尾部位置
			moveWrite(nLen);
			return true;
		}
		LOG_INFO("error, ByteStream::Write failed.");
		return false;
	}

	template<typename T>
	bool WriteArray(T* pData, uint32_t len)
	{
		//计算要写入数组的字节长度
		auto nLen = sizeof(T)*len;
		//判断能不能写入
		if (canWrite(nLen + sizeof(uint32_t)))
		{
			//先写入数组的元素数量
			Write(len);
			//将要写入的数据 拷贝到缓冲区尾部
			memcpy(_pBuff + _nWritePos, pData, nLen);
			//计算数据尾部位置
			push(nLen);
			return true;
		}
		LOG_INFO("error, ByteStream::WriteArray failed.");
		return false;
	}
};