/*
	�̰߳�ȫ��Ҫ������ȫ��һ����Ҫ���Ϊ�̰߳�ȫ�ĵ���ģʽ
*/

#ifndef _MEMORYMGE_H_
#define _MEMORYMGE_H_
#include<assert.h>
#include<mutex>
#include<iostream>

#define MAX_MEMORY_SZIE 1024 //����ڴ��Ϊ1024k
using namespace std;

class MemoryAlloc;

//�ڴ��������Ϣ����ʽ�洢
struct MemoryBlock
{
public:
	//�����ڴ��
	MemoryAlloc* pAlloc;
	//ָ����һ����ÿ�
	MemoryBlock* pNext;
	//�ڴ����
	int nID;
	//���ü���
	int nRef;
	//�Ƿ����ڴ����
	bool bPool;
private:
	//�ֽڶ���ԭ��Ԥ������ռ���ڲ���Ƭ
	char a;
	char b;
	char c;
};

//�ڴ�ػ������
class MemoryAlloc
{
protected:
	//�ڴ�ص�ַ
	char* _pAdr;
	//ͷ���ڴ浥Ԫ
	MemoryBlock* _pHeader;
	//�ڴ浥Ԫ��Ĵ�С
	size_t _nSzie;
	//�ڴ浥Ԫ������
	size_t _nBlockSzie;
	//�߳���
	std::mutex _mutex;
public:
	MemoryAlloc()
	{
		_pAdr = nullptr;
		_pHeader = nullptr;
		_nSzie = 0;
		_nBlockSzie = 0;
	}
	~MemoryAlloc()
	{
		if(nullptr != _pAdr)
			free(_pAdr);
	}

	//��ʼ���ڴ��Ϊ��ʽ�ṹ
	void initMemory()
	{
		if (nullptr != _pAdr)
			return;
		//�ڴ�ش�СΪ ��������Ϣ+�洢��Ԫ��* ����
		size_t realSzie = _nSzie + sizeof(MemoryBlock);
		size_t poolSize = realSzie*_nBlockSzie;
		//��ϵͳ�����ڴ�ռ�,ת��Ϊchar*���ͣ����ֽڲ��������ڲ���
		_pAdr = (char*)malloc(poolSize);
		//��һ��Ƭ�ڴ�ռ���Ƭ����,��ʼ��ͷ������Ϣ
		_pHeader = (MemoryBlock*)_pAdr;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pAlloc = this;
		_pHeader->pNext = nullptr;
		//������Ƭ����ʼ���ڴ��
		MemoryBlock* p1 = _pHeader;
		for (size_t i = 1; i < _nBlockSzie; i++)
		{
			MemoryBlock* p2 = (MemoryBlock*)(_pAdr + (i * realSzie));
			p2->bPool = true;
			p2->nID = i;
			p2->nRef = 0;
			p2->pAlloc = this;
			p2->pNext = nullptr;
			p1->pNext = p2;
			p1 = p2;
		}
	}

	//�����ڴ�
	void* allocMemory(size_t nSize)
	{
		//����
		std::lock_guard<std::mutex> mml(_mutex);
		if (nullptr == _pAdr)
		{
			initMemory();
		}

		MemoryBlock* pReturn = nullptr;

		//�ڴ����û�п��õ��ڴ��ʱֱ����ϵͳ����
		if (nullptr == _pHeader)
		{
			pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
			cout << "��ϵͳ�����룺" << nSize + sizeof(MemoryBlock) << "KB" << endl;
		}
		else 
		{
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			//�˿��Ƿ�ʹ�ù�
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
			cout << "���ڴ�أ�" << pReturn->pAlloc <<" ���룺" << pReturn->pAlloc->_nSzie <<  "KB" << endl;
		}
		//���ص�ַΪ�ڴ������������Ϣ�Ĳ���
		return ((char*)pReturn + sizeof(MemoryBlock));
	}

	//�ͷ��ڴ�
	void freeMemory(void* pMem)
	{
		//�û�ʹ���ڴ�δ����������Ϣ��ǰ��
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		assert(1 == pBlock->nRef);
		//�ÿ��ڴ��Ƿ����ڴ���зֱ���
		if (pBlock->bPool)
		{
			std::lock_guard<std::mutex> mml(_mutex);
			//���ü�����û���û�ʹ�ò��ͷ�
			if (--pBlock->nRef != 0)
			{
				return;
			}
			//ͷ�巨�����յ��ڴ���ڣ���Ϊ��ͷ
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else 
		{
			if (--pBlock->nRef != 0)
			{
				return;
			}
			free(pBlock);
		}
	}
};

//�ڴ������������ģ������ʼ��
template<size_t nSzie, size_t nBlockSzie>
class MemoryAlloctor :public MemoryAlloc
{
public:
	MemoryAlloctor()
	{
		//64λ��32λϵͳ�ֽڶ��뷽ʽ���죬��void*��ʶ��ϵͳ�������
		const size_t n = sizeof(void*);
		//�����������ڴ���С�����ֽڶ��� 
		_nSzie = (nSzie / n)*n + (nSzie % n ? n : 0);
		_nBlockSzie = nBlockSzie;
	}
};

//�ڴ����new��memorypool���м��,�Բ�ͬ�ߴ���ڴ�ؽ���hash����
class MemoryMge
{
public:
	//����ģʽ ��̬
	static MemoryMge& getMemoryMgeInstance()
	{	
		static MemoryMge mge;
		return mge;
	}

	//�����ڴ�
	void* allocMem(size_t nSize)
	{
		if (nSize <= MAX_MEMORY_SZIE)
		{
			return _szAlloc[nSize]->allocMemory(nSize);
		}
		else
		{
			MemoryBlock* pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
			cout << "��ϵͳ�����룺" << nSize + sizeof(MemoryBlock) << "KB" << endl;
			return ((char*)pReturn + sizeof(MemoryBlock));
		}
	}

	//�ͷ��ڴ�
	void freeMem(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		if (pBlock->bPool)
		{
			pBlock->pAlloc->freeMemory(pMem);
		}
		else
		{
			if (--pBlock->nRef == 0)
				free(pBlock);
		}
	}

	//�����ڴ������ü���
	void addRef(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		++pBlock->nRef;
	}

private:
	MemoryAlloctor<64, 100000> _mem64;
	MemoryAlloctor<128, 100000> _mem128;
	MemoryAlloctor<256, 100000> _mem256;
	MemoryAlloctor<512, 100000> _mem512;
	MemoryAlloctor<1024, 100000> _mem1024;
	//0~64 65~128 129~256 257~512 513~1024
	MemoryAlloc* _szAlloc[MAX_MEMORY_SZIE + 1];

	//��ʼ���ڴ��ӳ������
	void init_szAlloc(int nBegin, int nEnd, MemoryAlloc* pMemA)
	{
		for (int n = nBegin; n <= nEnd; n++)
		{
			_szAlloc[n] = pMemA;
		}
	}

	MemoryMge()
	{
		init_szAlloc(0, 64, &_mem64);
		init_szAlloc(65, 128, &_mem128);
		init_szAlloc(129, 256, &_mem256);
		init_szAlloc(257, 512, &_mem512);
		init_szAlloc(513, 1024, &_mem1024);
	}

	~MemoryMge()
	{

	}
};


#endif //_MEMORYMGE_H_