#ifndef _CELL_MSG_STREAM_HPP_
#define _CELL_MSG_STREAM_HPP_

#include"DataHeader.hpp"
#include"ByteStream.hpp"

//消息数据字节流
class ReadStream :public ByteStream
{
public:
	ReadStream(DataHeader* header)
		:ReadStream((char*)header, header->dataLength)
	{

	}

	ReadStream(char* pData, int nSize, bool bDelete = false)
		:ByteStream(pData, nSize, bDelete)
	{
		moveRead(nSize);
	}

	uint16_t getNetCmd()
	{
		uint16_t cmd = CMD_ERROR;
		Read<uint16_t>(cmd);
		return cmd;
	}
};

//消息数据字节流
class WriteStream :public ByteStream
{
public:
	WriteStream(char* pData, int nSize, bool bDelete = false)
		:ByteStream(pData, nSize, bDelete)
	{
		Write<uint16_t>(0);
	}

	WriteStream(int nSize = 1024)
		:ByteStream(nSize)
	{
		Write<uint16_t>(0);
	}

	void setNetCmd(uint16_t cmd)
	{
		Write<uint16_t>(cmd);
	}

	bool WriteString(const char* str, int len)
	{
		return WriteArray(str, len);
	}

	bool WriteString(const char* str)
	{
		return WriteArray(str, strlen(str));
	}

	bool WriteString(std::string& str)
	{
		return WriteArray(str.c_str(), str.length());
	}
	void finsh()
	{
		int pos = getLength();
		setWritePos(0);
		Write<uint16_t>(pos);
		setWritePos(pos);
	}
};


#endif // !_CELL_MSG_STREAM_HPP_
